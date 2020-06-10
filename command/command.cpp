/* 
 * MeSQL - Command - command.cpp
 * 
 * This file implements the classes defined in command.hpp .
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include "../base/base.hpp"
#include "../base/color.hpp"
#include "../base/timer.hpp"
#include "../base/error.hpp"
#include "../index/index.hpp"
#include "../interpreter/interpreter.hpp"
#include "command.hpp"

using namespace std;

static const COLOR default_color = COLOR::blue;

namespace MeInt {
    using namespace MeType;
    using namespace MeInfo;
	using namespace MeTime;
	using namespace MeError;
	using namespace MeInd;

    // implement class Statement
    Statement::Statement() : content(),printer(cout) {} // note this printer initialization
	void Statement::execute() {
		Timer t; t.start();
		try {
			_execute();
			t.stop(); printer << t.paren_str() << endl;
		} catch (const MeError::MeError &e) {
			printer << e.str() << endl;
		} catch (const exception &e) {
			printer << e.what() << endl;
		}
	}
	void Statement::set_manager(Manager *_man) {
		man = _man;
	}

    // implement class CreateTableStatement
    CreateTableStatement::CreateTableStatement(const string &_table_name,const vector<TableColumnDef> &_cols,const string &_primary_key) :
        table_name(_table_name),cols(_cols),primary_key(_primary_key) {
		give_order();
	}

    string CreateTableStatement::str() const {
        static stringstream ss;
		ss.str("");
        ss << colorful("create table statement :",default_color) << endl;
        ss << "table name : " << table_name << endl;
        ss << "columns definition :" << endl;
        for (const TableColumnDef &c:cols) ss << '\t' << c.str() << endl;
        ss << "primary key : " << primary_key << endl;
        return ss.str();
    }
    void CreateTableStatement::print() const {
        printer << str() << endl;
    }
    void CreateTableStatement::_execute() { // do a lot of check
		// Assume by lexer & parser , the identifiers are valid identifier

		// check table_name
		if (man->cat.tables.count(table_name)) throw MeError::MeError(
			"Invalid Operation",
			"table '" + table_name + "' already exists"
		);

		// check cols
		size_t col_num = cols.size();
		if (col_num == 0) throw MeError::MeError(
			"Invalid Table Definition",
			"no columns in table"
		);
		if (col_num > max_col_num) throw MeError::MeError(
			"Invalid Table Definition",
			"too many (" + to_str(col_num) + " > " + to_str(max_col_num) + ") columns"
		);
		// assume by parser.y implementation
		for (const TableColumnDef &def:cols) 
			if (def.col_spec.data_type == DataType::CHAR) assert(1<=def.col_spec.len && def.col_spec.len<=255);

		pair<size_t,size_t> l = calc_len(cols);
		if (l.second > block_size) throw MeError::MeError(
			"Invalid Table Definition",
			"tuple total len = " + to_str(l.second) + " cannot fit into a block"
		);
		vector<string> names(col_num);
		for (size_t i=0;i<col_num;++i) names[i] = cols[i].col_name;
		if (!MeGad::check_unique(names)) throw MeError::MeError(
			"Invalid Table Definition",
			"repeat column name"
		);

		// check and set primary key
		vector<string>::iterator it = find(names.begin(),names.end(),primary_key);
		if (it == names.end()) throw MeError::MeError(
			"Invalid Table Definition",
			"primary key not in column list"
		);
		col_num_t pk = it - names.begin();
		cols[pk].col_spec.is_primary_key = true;
		cols[pk].col_spec.is_unique = true;

		// create table catalog and file
		TableInfo &ti = man->cat.create_table_catalog(table_name,cols,pk);
		Recorder rec(*man,ti);
		rec.init_table();

		// create primary index
		string idn = table_name + "_PI";
		if (man->cat.indexes.count(idn) == 1) throw MeError::MeError(
			"Bad Situation",
			"primary index name '" + idn + "' for table '" + table_name + "' already exists. Please drop it and then create this table"
		);

		__create_index(*man,table_name + "_PI",table_name,pk);

		printer << "OK ";
    }
    void CreateTableStatement::give_order() {
        for (size_t i=0;i<cols.size();++i) cols[i].ord = i;
    }

    // implement class DropTableStatement
    DropTableStatement::DropTableStatement(const string &_table_name) : table_name(_table_name) {}
    string DropTableStatement::str() const {
        static stringstream ss;
		ss.str("");
        ss << colorful("drop table statement :",default_color) << endl;
        ss << "table name : " << table_name << endl;
        return ss.str();
    }
    void DropTableStatement::print() const {
        printer << str() << endl;
    }
    void DropTableStatement::_execute() {
		if (man->cat.tables.count(table_name) == 0) throw MeError::MeError(
			"Invalid Drop Table Statement",
			"table '" + table_name + "' not exists"
		);

		TableInfo &ti = man->cat.tables.at(table_name);
		
		// first remove all indexes on this table
		const auto &st = ti.index_on;
		vector<string> related_index(st.begin(),st.end());
		for (const string &s:related_index) __drop_index(*man,s);

		Recorder rec(*man,man->cat.tables.at(table_name));
		rec.remove_table();
		man->cat.erase_table_catalog(table_name);

		printer << "OK ";
    }

    // implement class CreateIndexStatement
    CreateIndexStatement::CreateIndexStatement(const string &_index_name,const string &_table_name,const string &_col_name) :
        index_name(_index_name),table_name(_table_name),col_name(_col_name) {}
    string CreateIndexStatement::str() const {
        static stringstream ss;
		ss.str("");
        ss << colorful("create index statement :",default_color) << endl;
        ss << "index name : " << index_name << endl;
        ss << "table name : " << table_name << endl;
        ss << "column name : " << col_name << endl;
        return ss.str();
    }
    void CreateIndexStatement::print() const {
        printer << str() << endl;
    }
    void CreateIndexStatement::_execute() {
		if (man->cat.indexes.count(index_name) == 1) throw MeError::MeError(
			"Invalid Create Index Statement",
			"index '" + index_name + "' already exists"
		);
		if (man->cat.tables.count(table_name) == 0) throw MeError::MeError(
			"Invalid Create Index Statement",
			"table '" + table_name + "' not exists"
		);
		TableInfo &ti = man->cat.tables.at(table_name);
		col_num_t ord = -1;
		for (const TableColumnDef &def:ti.def.col_def) if (def.col_name == col_name) {
			ord = def.ord;
			break;
		}
		if (ord == -1) throw MeError::MeError(
			"Invalid Create Index Statement",
			"column '" + col_name + "' not in table '" + table_name + "'"
		);

		__create_index(*man,index_name,table_name,ord);

		printer << "OK ";
    }

    // implement class DropIndexStatement
    DropIndexStatement::DropIndexStatement(const string &_index_name) : index_name(_index_name) {}
    string DropIndexStatement::str() const {
        static stringstream ss;
		ss.str("");
        ss << colorful("drop index statement :",default_color) << endl;
        ss << "index name : " << index_name << endl;
        return ss.str();
    }
    void DropIndexStatement::print() const {
        printer << str() << endl;
    }
    void DropIndexStatement::_execute() {
		if (man->cat.indexes.count(index_name) == 0) throw MeError::MeError(
			"Invalid Drop Index Statement",
			"index '" + index_name + "' not exists"
		);
		
		__drop_index(*man,index_name);

		printer << "OK ";
    }

    // implement class SelectStatement
    SelectStatement::SelectStatement(const string &_table_name,const vector<string> &_proj_cols,const WhereCond &_cond) :
        table_name(_table_name),proj_cols(_proj_cols),projs(),cond(_cond) {}
    string SelectStatement::str() const {
        static stringstream ss;
		ss.str("");
        ss << colorful("select statement :",default_color) << endl;
        ss << "table name : " << table_name << endl;
        ss << "project to columns : ";
        for (const string &col_name:proj_cols) ss << col_name << ' ';
        ss << endl;
        ss << "where conditions : " << cond.str() << endl;
        return ss.str();
    }
    void SelectStatement::print() const {
        printer << str() << endl;
    }
    void SelectStatement::_execute() {
		// check table_name
		if (man->cat.tables.count(table_name) == 0) throw MeError::MeError(
			"Invalid Select Statement",
			"table '" + table_name + "' not exists"
		);
		TableInfo &ti = man->cat.tables.at(table_name);

		// check cond
		assert(cond.otems.empty());
		size_t cons = cond.items.size();
		for (size_t i=0;i<cons;++i) {
			const WhereCondItem &item = cond.items.at(i);
			if (ti.name_to_ord.count(item.col_name) == 0) throw MeError::MeError(
				"Invalid Select Statement",
				"column '" + item.col_name + "' not in table '" + table_name + "'"
			);
			col_num_t ord = ti.name_to_ord.at(item.col_name);
			DataType fir = ti.def.col_def.at(ord).col_spec.data_type;
			DataType sec = item.lit.dtype;
			// comparability check
			if (fir != sec) throw MeError::MeError(
				"Invalid Select Statement",
				"cannot compare column '" + item.col_name + "'(" + DataTypeStr(fir) + ") "
				"and literal [" + item.lit.str() + "](" + DataTypeStr(sec) + ")"
			);
			cond.otems.emplace_back(ord,item.op,item.lit);
		}

		// check proj_cols
		size_t psiz = proj_cols.size();
		assert(psiz > 0);
		assert(projs.empty());
		for (const string &s:proj_cols) {
			if (s == "*") {
				for (col_num_t j=0;j<ti.def.col_def.size();++j) projs.push_back(j);
			} else {
				if (ti.name_to_ord.count(s) == 0) throw MeError::MeError(
					"Invalid Select Statement",
					"project column '" + s + "' not exists"
				);
				projs.push_back(ti.name_to_ord.at(s));
			}
		}

		Lister<size_t> l = __do_select(*man,ti,cond,true);
		if (l.num == 0) {
			printer << "empty set ";
			l.dest();
			return;
		}
		Recorder rec(*man,ti);
		Resulter res(*man);
		
		vector<col_num_t> aux = projs;
		aux.erase(unique(aux.begin(),aux.end()),aux.end());
		for (col_num_t &c:projs) c = lower_bound(aux.begin(),aux.end(),c) - aux.begin();
		vector<TableColumnDef> tcd;
		for (col_num_t c:aux) tcd.emplace_back(ti.def.col_def.at(c));
		res.init(tcd);
		l.start(false);
		for (size_t i=0;i<l.num;++i) {
			size_t pos = l.pop_front();
			const vector<Literal> &got = rec.get_record(pos);
			vector<Literal> tup;
			for (col_num_t c:aux) tup.push_back(got.at(c));
			res.add_tuple(tup);
		}
		l.finish();
		l.dest();
		vector<pair<size_t,string>> prt_spec;
		for (col_num_t c:projs) prt_spec.emplace_back(c,"");
		res.print(printer,prt_spec);
    }

    // implement class InsertStatement
    InsertStatement::InsertStatement(const string &_table_name,const InsertTuples &_tps) :
        table_name(_table_name),tps(_tps) {}

    string InsertStatement::str() const {
        static stringstream ss;
		ss.str("");
        ss << colorful("insert statement :",default_color) << endl;
        ss << "table name : " << table_name << endl;
        ss << "insert tuples :" << endl;
        for (const InsertTuple &tup:tps) ss << '\t' << tup.str() << endl;
        return ss.str();
    }
    void InsertStatement::print() const {
        printer << str() << endl;
    }
    void InsertStatement::_execute() {
		size_t siz = tps.size();
		assert(siz > 0);
		if (man->cat.tables.count(table_name) == 0) throw MeError::MeError(
			"Invalid Insert Statement",
			"table '" + table_name + "' does not exist"
		);
		TableInfo &ti = man->cat.tables.at(table_name);
		for (const InsertTuple &tup:tps) if (!ti.fit_tuple(tup)) throw MeError::MeError(
			"Invalid Insert Tuple",
			"tuple " + tup.str() + " does not fit into this table"
		);
		
		size_t ids = ti.index_on.size();
		vector<Indexer> ind;
		ind.reserve(ids);
		for (const string &idx:ti.index_on) {
			IndexInfo &di = man->cat.indexes.at(idx);
			ind.emplace_back(di);
		}
		Recorder rec(*man,ti);
		for (const InsertTuple &tup:tps) {
			bool t = check_unique_constraint(*man,ti,ind,rec,tup);
			if (t == false) throw MeError::MeError(
				"Integrity Constraint Violated",
				"tuple " + tup.str() + " violated at least one uniqueness constraint"
			);
		}
		for (const InsertTuple &tup:tps) {
			size_t pos = rec.place_record(tup);
			for (Indexer &ide:ind) ide.insert_record(tup,pos);
		}

		printer << "OK " << siz << " " << (siz == 1 ? "tuple" : "tuples") << " inserted ";
    }

    // implement class DeleteStatement
    DeleteStatement::DeleteStatement(const string &_table_name,const WhereCond &_cond) :
        table_name(_table_name),cond(_cond) {}

    string DeleteStatement::str() const {
        static stringstream ss;
		ss.str("");
        ss << colorful("delte statement :",default_color) << endl;
        ss << "table name : " << table_name << endl;
        ss << "where condition : " << cond.str() << endl;
        return ss.str();
    }
    void DeleteStatement::print() const {
        printer << str() << endl;
    }
    void DeleteStatement::_execute() {
		if (man->cat.tables.count(table_name) == 0) throw MeError::MeError(
			"Invalid Delet Statement",
			"table '" + table_name + "' not exists"
		);
		TableInfo &ti = man->cat.tables.at(table_name);

		// check cond
		assert(cond.otems.empty());
		size_t cons = cond.items.size();
		for (size_t i=0;i<cons;++i) {
			const WhereCondItem &item = cond.items.at(i);
			if (ti.name_to_ord.count(item.col_name) == 0) throw MeError::MeError(
				"Invalid Delete Statement",
				"column '" + item.col_name + "' not in table '" + table_name + "'"
			);
			col_num_t ord = ti.name_to_ord.at(item.col_name);
			DataType fir = ti.def.col_def.at(ord).col_spec.data_type;
			DataType sec = item.lit.dtype;
			// comparability check
			if (fir != sec) throw MeError::MeError(
				"Invalid Delete Statement",
				"cannot compare column '" + item.col_name + "'(" + DataTypeStr(fir) + ") "
				"and literal [" + item.lit.str() + "](" + DataTypeStr(sec) + ")"
			);
			cond.otems.emplace_back(ord,item.op,item.lit);
		}

		Lister<size_t> l = __do_select(*man,ti,cond,true);
		if (l.num > 0) {
			Recorder rec(*man,ti);
			vector<Indexer> idn;
			for (const string &s:ti.index_on) idn.emplace_back(man->cat.indexes.at(s));
			l.start(false);
			for (size_t i=0;i<l.num;++i) {
				size_t pos = l.pop_front();
				vector<Literal> tup = rec.erase_record_at(pos);
				for (Indexer &ide:idn) ide.delete_record(tup,pos);
			}
			l.finish();
		}
		l.dest();
		printer << "OK " << l.num << " rows deleted ";
    }

    // implement class ExecfileStatement
    ExecfileStatement::ExecfileStatement(const string &_file_name) : file_name(_file_name) {}

    string ExecfileStatement::str() const {
        static stringstream ss;
		ss.str("");
        ss << colorful("execfile statement :",default_color) << endl;
        ss << "file name : " << file_name << endl;
        return ss.str();
    }
    void ExecfileStatement::print() const {
        printer << str() << endl;
    }
    void ExecfileStatement::_execute() {
		ifstream f(file_name);
		if (f.fail()) throw MeError::MeError(
			"Invalide Execfile Statement",
			"file '" + file_name + "' cannot be opened (maybe not exists)"
		);
		Interpreter o(&f,&printer,false,*man);
		o.parse();

		printer << "Execfile Finish ";
    }

	// implement class ShowTablesStatement
	ShowTablesStatement::ShowTablesStatement() {}
	string ShowTablesStatement::str() const {
		return colorful("show tables statement",default_color);
	}
	void ShowTablesStatement::print() const {
		printer << str() << endl;
	}
	void ShowTablesStatement::_execute() {
		const map<string,TableInfo> &mp = man->cat.tables;
		Resulter res = man->new_resulter();
		res.init({TableColumnDef("tables",DataType::CHAR,max_CHAR_len)});
		for (const pair<string,TableInfo> &pr:mp) res.add_tuple({Literal(pr.first)});
		res.print(cout);
		res.finish();
	}

	// implement class ShowIndexesStatement
	ShowIndexesStatement::ShowIndexesStatement() {}
	string ShowIndexesStatement::str() const {
		return colorful("show indexes statement",default_color);
	}
	void ShowIndexesStatement::print() const {
		printer << str() << endl;
	}
	void ShowIndexesStatement::_execute() {
		const map<string,TableInfo> &tab = man->cat.tables;
		const map<string,IndexInfo> &ind = man->cat.indexes;
		Resulter res = man->new_resulter();
		res.init({
			TableColumnDef("index name",DataType::CHAR,max_CHAR_len),
			TableColumnDef("table",DataType::CHAR,max_CHAR_len),
			TableColumnDef("column",DataType::CHAR,max_CHAR_len)
		});
		for (const pair<string,IndexInfo> &pr:ind) {
			const string &col = tab.at(pr.second.def.table_name).def.col_def[pr.second.def.col_ord].col_name;
			res.add_tuple({
				Literal(pr.first),Literal(pr.second.def.table_name),Literal(col)
			});
		}
		res.print(cout);
		res.finish();
	}

	Lister<size_t> __do_select(Manager &man,TableInfo &ti,WhereCond &cond,bool logic_and) {
		if (cond.items.empty()) return __do_select_with_brute(man,ti,cond,logic_and);

		size_t siz = cond.items.size();
		assert(siz == cond.otems.size());
		map<col_num_t,const IndexInfo &> h_ind;
		for (const string &s:ti.index_on) {
			const IndexInfo &di = man.cat.indexes.at(s);
			h_ind.emplace(di.col.ord,di);
		}
		for (size_t i=0;i<siz;++i) {
			WhereCondOtem &ot = cond.otems.at(i);
			ot.has_index = h_ind.count(ot.col_ord);
			if (ot.has_index && ot.op == CompareOp::EQ) {
				const IndexInfo &di = h_ind.at(ot.col_ord);
				swap(cond.items.front(),cond.items.at(i));
				swap(cond.otems.front(),cond.otems.at(i));
				return __do_select_with_index(man,ti,cond,di,logic_and);
			}
		}
		for (size_t i=0;i<siz;++i) {
			WhereCondOtem &ot = cond.otems.at(i);
			if (ot.has_index && ot.op != CompareOp::NE) {
				const IndexInfo &di = h_ind.at(ot.col_ord);
				swap(cond.items.front(),cond.items.at(i));
				swap(cond.otems.front(),cond.otems.at(i));
				return __do_select_with_index(man,ti,cond,di,logic_and);
			}
		}
		return __do_select_with_brute(man,ti,cond,logic_and);
	}

	Lister<size_t> __do_select_with_brute(Manager &man,TableInfo &ti,WhereCond &cond,bool logic_and) {
		Recorder rec(man,ti);
		Recorder::RecNode r(rec.header());
		Lister<size_t> l(man);
		l.init();
		l.start(true);
		while (r.p.nxt_tup) {
			size_t pos = r.p.nxt_tup;
			r = rec.get_recnode(pos);
			if (cond.satisfy(r.tup,logic_and)) l.push_back(pos);
		}
		l.finish();
		return l;
	}

	Lister<size_t> __do_select_with_index(Manager &man,TableInfo &ti,WhereCond &cond,IndexInfo &di,bool logic_and) {
		Indexer ide(man,di);
		Recorder rec(man,ti);
		Lister<size_t> l(man);
		l.init();
		l.start(true);
		pair<BNode,size_t> pos(0,0);
		CompareOp dop = cond.otems.front().op;
		CompareOp oop = dop;
		const Literal &fli = cond.otems.front().lit;
		if (dop == CompareOp::EQ) dop = CompareOp::GE;
		if (dop == CompareOp::L || dop == CompareOp::LE) pos = ide.first_leaf_start(); else
		if (dop == CompareOp::G) pos = ide.first_greater_pos(fli); else
		if (dop == CompareOp::GE) pos = ide.first_greater_equal_pos(fli);
		bool flag = true;
		while (pos.first.seg) {
			for (;pos.second < pos.first.K.size();++pos.second) {
				const Literal &col_val = pos.first.K.at(pos.second);
				if (!CompareOpApply(col_val,oop,fli)) {
					flag = false;
					break;
				}
				size_t fp = pos.first.P.at(pos.second);
				Recorder::RecNode r = rec.get_recnode(fp);
				if (cond.satisfy(r.tup,logic_and)) l.push_back(fp);
			} 
			if (!flag) break;
			size_t nxt_node = pos.first.P.back();
			pos.first = ide.get_bnode(nxt_node);
			pos.second = 0;
		}
		l.finish();
		return l;
	}
	
	bool check_unique_constraint(Manager &man,TableInfo &ti,vector<Indexer> &ind,Recorder &rec,const InsertTuple &tup) {
		WhereCond cond;
		for (const TableColumnDef &def:ti.def.col_def) if (def.col_spec.is_unique) {
			cond.items.emplace_back(def.col_name,CompareOp::EQ,tup.at(def.ord));
			cond.otems.emplace_back(def.ord,CompareOp::EQ,tup.at(def.ord));
		}
		Lister<size_t> l = __do_select(man,ti,cond,false);
		size_t num = l.num;
		l.dest();
		return num == 0;
	}

	void __create_index(Manager &man,const string &index_name,const string &table_name,col_num_t ord) {
		TableInfo &ti = man.cat.tables.at(table_name);
		IndexInfo &di = man.cat.create_index_catalog(index_name,ti,ord);
		Indexer idn(man,di);
		idn.init_index();

		// insert record into newly created index
		Recorder rec(man,ti);
		Recorder::RecNode r(rec.header());
		while (r.p.nxt_tup) {
			size_t pos = r.p.nxt_tup;
			vector<Literal> val = rec.get_record(pos);
			idn.insert_record(val,pos);
		}
	}
	void __drop_index(Manager &man,const string &index_name) {
		IndexInfo di = man.cat.indexes.at(index_name);
		Indexer idn(man,di);
		idn.remove_index();
		man.cat.erase_index_catalog(index_name);
	}
}