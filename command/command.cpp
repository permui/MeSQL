/* 
 * MeSQL - Command - command.cpp
 * 
 * This file implements the classes defined in command.hpp .
 */


#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../base/base.hpp"
#include "../base/color.hpp"
#include "../base/timer.hpp"
#include "../base/error.hpp"
#include "command.hpp"

using namespace std;

static const COLOR default_color = COLOR::blue;

namespace MeInt {
    using namespace MeType;
    using namespace MeInfo;
	using namespace MeTime;
	using namespace MeError;

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
		if (col_num > 32) throw MeError::MeError(
			"Invalid Table Definition",
			"too many (" + to_str(col_num) + " > 32) columns"
		);
		pair<size_t,size_t> l = calc_len(cols);
		if (l.second > block_size) throw MeError::MeError(
			"Invalid Table Definition",
			"tuple total len = " + to_str(l.second) + " cannot fit into a block"
		);
		vector<string> names(col_num);
		for (size_t i=0;i<col_num;++i) names[i] = cols[i].col_name;

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
    void DropTableStatement::execute() {
        // TODO
        // now just print
        print();
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
    void CreateIndexStatement::execute() {
        // TODO
        // now just print
        print();
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
    void DropIndexStatement::execute() {
        // TODO
        // now just print
        print();
    }

    // implement class SelectStatement
    SelectStatement::SelectStatement(const string &_table_name,const vector<string> &_proj_cols,const WhereCond &_cond) :
        table_name(_table_name),proj_cols(_proj_cols),cond(_cond) {}
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
    void SelectStatement::execute() {
        // TODO
        // now just print
        print();
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
    void InsertStatement::execute() {
        // TODO
        // now just print
        print();
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
    void DeleteStatement::execute() {
        // TODO
        // now just print
        print();
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
    void ExecfileStatement::execute() {
        // TODO
        // now just print
        print();
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
			TableColumnDef("name",DataType::CHAR,max_CHAR_len),
			TableColumnDef("for table",DataType::CHAR,max_CHAR_len),
			TableColumnDef("on column",DataType::CHAR,max_CHAR_len)
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
}