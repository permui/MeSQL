/*
 * MeSQL - base - manager.cpp
 * 
 * This file defines the class declared in manager.hpp . 
 * 
 */

#include <sstream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <algorithm>
#include "manager.hpp"
#include "error.hpp"
#include "../config.hpp"

static const char VERT = '|';
static const char HORI = '-';
static const char CORN = '+';

namespace MeMan {

	template<typename T> void Recorder::embed(const T &val,size_t len,Block &blo) {
		throw MeError::MeError(
			InternalError,
			"unspecialized Recorder::embed should not be called"
		);
	}
	template<typename T> void Recorder::parse(T &val,size_t len,Block &blo) {
		throw MeError::MeError(
			InternalError,
			"unspecialized Recorder::parse should not be called"
		);
	}
	template<> void Recorder::embed<Literal>(const Literal &val,size_t len,Block &blo) {
		switch (val.dtype) {
			case DataType::INT : {
				assert(len == sizeof (val.int_val));
				blo.write(val.int_val);
				break;
			}
			case DataType::FLOAT : {
				assert(len == sizeof (val.float_val));
				blo.write(val.float_val);
				break;
			}
			case DataType::CHAR : {
				size_t siz = val.char_val.length();
				assert(len >= siz);
				blo.raw_write(val.char_val.c_str(),siz);
				blo.raw_fill(0,len-siz);
				break;
			}
		}
	}
	template<> void Recorder::parse<Literal>(Literal &val,size_t len,Block &blo) {
		switch (val.dtype) {
			case DataType::INT : {
				assert(len == sizeof (val.int_val));
				int x;
				blo.read(x);
				val = Literal(x);
				break;
			}
			case DataType::FLOAT : {
				assert(len == sizeof (val.float_val));
				float x;
				blo.read(x);
				val = Literal(x);
				break;
			}
			case DataType::CHAR : {
				static char aux[block_size],*r;
				blo.raw_read(aux,len);
				for (r=aux+len;r!=aux && *(r-1)==0;--r);
				val = Literal(string(aux,r));
				break;
			}
		}
	}

}

namespace MeMan {
	using namespace MeBuf;

	// implement class TmpManager
	TmpManager::TmpManager(Manager &_man) : man(_man),cnt(0) {}
	TmpManager::~TmpManager() {
		for (size_t i=1;i<=cnt;++i) remove(tmp_name(i).c_str());
	}
	string TmpManager::tmp_name(size_t x) const {
		static stringstream ss;
		ss.str("");
		ss << DB_FILES "tmp/" << x << ".tmp";
		return ss.str();
	}
	string TmpManager::new_tmp() {
		string ret = tmp_name(++cnt);
		return ret;
	}
	void TmpManager::ret_tmp(const string &s) {
		man.buf.remove_file(s);
		remove(s.c_str()); // delete the file
	}

	// implement class Resulter
	Resulter::Resulter(Manager &_man) : 
		man(_man),col_defs(),col_width(),col_cnt(0),row_cnt(0),file(""),
		writer_block(0),writer_pos(0),reader_block(0),reader_pos(0) {}
	void Resulter::init(const vector<TableColumnDef> &c) {
		assert(!c.empty());
		col_defs = c;
		col_cnt = col_defs.size();
		col_width.assign(col_cnt,0);
		for (size_t i=0;i<col_cnt;++i) {
			TableColumnDef &def = col_defs.at(i);
			int l = DataTypeLen(def.col_spec.data_type);
			if (l != -1) def.col_spec.len = l;
			assert(def.col_spec.len != 0);
		}
		row_cnt = 0;
		file = man.tmp.new_tmp();
		writer_block = writer_pos = 0;
		reader_block = reader_pos = 0;
	}
	void Resulter::finish() {
		col_defs.clear();
		col_width.clear();
		col_cnt = row_cnt = 0;
		man.tmp.ret_tmp(file);
		file = "";
		writer_block = writer_pos = 0;
		reader_block = reader_pos = 0;
	}
	void Resulter::add_tuple(const vector<Literal> &tup) {
		assert(tup.size() == col_cnt);
		for (size_t i=0;i<col_cnt;++i) {
			assert(tup.at(i).dtype == col_defs.at(i).col_spec.data_type);
			if (tup[i].dtype == DataType::CHAR) 
				assert(tup[i].char_val.length() <= col_defs[i].col_spec.len);
		}
		++row_cnt;
		for (size_t i=0;i<col_cnt;++i) {
			write_item(tup[i],col_defs[i].col_spec.len);
			col_width[i] = max(col_width[i],(int)tup[i].str_unquoted().length());
		}
	}
	void Resulter::print(ostream &os) {
		if (row_cnt) {
			for (size_t i=0;i<col_cnt;++i) 
				col_width[i] = max(col_width[i],(int)col_defs[i].col_name.length());
			string hor = horizontal_line(col_cnt,col_width);
			os << hor << endl;
			os << VERT;
			for (size_t i=0;i<col_cnt;++i) {
				os << adjust(col_defs[i].col_name,col_width[i]);
				os << VERT;
			}
			os << endl;
			os << hor << endl;
			for (size_t i=0;i<row_cnt;++i) {
				os << VERT;
				for (size_t j=0;j<col_cnt;++j) {
					const Literal &lit = get_next_lit(col_defs[j].col_spec);
					os << adjust(lit.str_unquoted(),col_width[j]);
					os << VERT;
				}
				os << endl;
			}
			os << hor << endl;
			os << row_cnt << " rows in result ";
		} else {
			os << "empty result ";
		}
	}
	void Resulter::print(ostream &os,vector<pair<size_t,string>> prt_spec) {
		assert(!prt_spec.empty());
		if (row_cnt) {
			size_t _col_cnt = prt_spec.size();
			vector<int> _col_width(_col_cnt);
			for (size_t i=0;i<_col_cnt;++i) {
				assert(prt_spec[i].first < col_cnt);
				if (prt_spec[i].second.empty()) 
					prt_spec[i].second = col_defs[prt_spec[i].first].col_name;
				_col_width[i] = max(col_width[prt_spec[i].first],(int)prt_spec[i].second.length());
			}
			string hor = horizontal_line(_col_cnt,_col_width);
			os << hor << endl;
			os << VERT;
			for (size_t i=0;i<_col_cnt;++i) {
				os << adjust(prt_spec[i].second,_col_width[i]);
				os << VERT;
			}
			os << endl;
			os << hor << endl;
			static vector<Literal> tmp;
			tmp.resize(col_cnt);
			for (size_t i=0;i<row_cnt;++i) {
				for (size_t j=0;j<col_cnt;++j) tmp[j] = get_next_lit(col_defs[j].col_spec);
				os << VERT;
				for (size_t j=0;j<_col_cnt;++j) {
					os << adjust(tmp[prt_spec[j].first].str_unquoted(),_col_width[j]);
					os << VERT;
				}
				os << endl;
			}
			os << hor << endl;
			os << row_cnt << " rows in result ";
		} else {
			os << "empty result ";
		}
	}
	string Resulter::horizontal_line(size_t col_cnt,const vector<int> &col_width) const {
		string ret(1,CORN);
		for (size_t i=0;i<col_cnt;++i) {
			ret += string(col_width[i]+2,HORI);
			ret += CORN;
		}
		return ret;
	}
	void Resulter::write_item(const Literal &lit,size_t siz) {
		if (writer_pos + siz > block_size) ++writer_block,writer_pos = 0;
		Block blo = man.buf.get_block(file,writer_block,true);
		blo.ink();
		switch (lit.dtype) {
			case DataType::INT : 
				memcpy(blo.data + writer_pos,&lit.int_val,siz);
				break;
			case DataType::FLOAT :
				memcpy(blo.data + writer_pos,&lit.float_val,siz);
				break;
			case DataType::CHAR :
				memset(blo.data + writer_pos,0,siz);
				memcpy(blo.data + writer_pos,lit.char_val.c_str(),lit.char_val.length());
				break;
		}
		writer_pos += siz;
		blo.unpin();
	}
	string Resulter::adjust(string s,int width) {
		assert(s.length() <= width);
		return ' ' + s + string(width - s.length()+1,' ');
	}
	Literal Resulter::get_next_lit(const TableColumnSpec &spec) {
		size_t siz = spec.len;
		if (reader_pos + siz > block_size) ++reader_block,reader_pos = 0;
		Block blo = man.buf.get_block(file,reader_block,false);
		assert(blo.data);
		Literal ret;
		switch (spec.data_type) {
			case DataType::INT : {
				int x;
				memcpy(&x,blo.data + reader_pos,siz);
				ret = Literal(x);
				break;
			}
			case DataType::FLOAT : {
				float x;
				memcpy(&x,blo.data + reader_pos,siz);
				ret = Literal(x);
				break;
			}
			case DataType::CHAR : {
				char *l = blo.data + reader_pos;
				char *r = l + siz;
				while (l!=r && *(r-1)==0) --r;
				ret = Literal(string(l,r));
				break;
			}
		}
		reader_pos += siz;
		blo.unpin();
		return ret;
	}

	// implement class Recorder::ptr
	Recorder::ptr::ptr() : pre_tup(0),nxt_tup(0),nxt_spa(0) {}
	Recorder::ptr::ptr(size_t _pre_tup,size_t _nxt_tup,size_t _nxt_spa) :
		pre_tup(_pre_tup),nxt_tup(_nxt_tup),nxt_spa(_nxt_spa) {}

	// implement class Recorder::RecNode
	Recorder::RecNode::RecNode() : p(),tup() {}

	// implementm class Recorder
	Recorder::Recorder(Manager &_man,TableInfo &_ti) : man(_man),ti(_ti),in_len(0),out_len(0) {
		pair<size_t,size_t> l = calc_len(ti.def.col_def);
		in_len = l.first,out_len = l.second;
		if (out_len > block_size) throw MeError::MeError(
			InternalError,
			"cannot place tuple of table '" + ti.def.table_name + "' into one block, in_len = " + to_str(in_len)
		);
	}
	size_t Recorder::next_valid_pos(size_t pos) const {
		size_t seg = pos / block_size;
		size_t off = pos % block_size;
		if (off + out_len + out_len > block_size) return block_size * (seg + 1);
		return pos + out_len;
	}
	void Recorder::init_table() {
		ti.path = TABLE_DIR + ti.def.table_name + TABLE_SUF;
		Block blo = man.buf.get_block(ti.path,0,true);
		blo.ink();
		ptr init_ptr(0,0,block_size);
		blo.seek(0),blo.write(init_ptr);
		blo.unpin();
	}
	void Recorder::remove_table() {
		man.buf.remove_file(ti.path);
		remove(ti.path.c_str());
	}
	Recorder::RecNode Recorder::header() {
		Block head = man.buf.get_block(ti.path,0,false);
		assert(head.data);
		RecNode ret;
		head.seek(0),head.read(ret.p);
		head.unpin();
		return ret;
	}
	Recorder::RecNode Recorder::get_recnode(size_t pt) {
		size_t seg = pt / block_size;
		size_t off = pt % block_size;
		Block the = man.buf.get_block(ti.path,seg,false);
		assert(the.data);
		the.seek(off);
		RecNode ret;
		the.read(ret.p);
		size_t siz = ti.def.col_def.size();
		ret.tup.resize(siz);
		for (size_t i=0;i<siz;++i) {
			ret.tup.at(i).dtype = ti.def.col_def.at(i).col_spec.data_type;
			parse(ret.tup.at(i),ti.def.col_def.at(i).col_spec.len,the);
		}
		the.unpin();
		return ret;
	}
	vector<Literal> Recorder::get_record(size_t pt) {
		size_t seg = pt / block_size;
		size_t off = pt % block_size;
		Block the = man.buf.get_block(ti.path,seg,false);
		assert(the.data);
		the.seek(off);
		the.fake<ptr>();
		size_t siz = ti.def.col_def.size();
		vector<Literal> ret(siz);
		for (size_t i=0;i<siz;++i) {
			ret.at(i).dtype = ti.def.col_def.at(i).col_spec.data_type;
			parse(ret.at(i),ti.def.col_def.at(i).col_spec.len,the);
		}
		the.unpin();
		return ret;
	}
	size_t Recorder::place_record(vector<Literal> &rec) {
		if (!ti.try_to_fit_tuple(rec)) throw MeError::MeError(
			InternalError,
			"Recorder::place_record got unfit tuple"
		);
		Block head = man.buf.get_block(ti.path,0,false);
		head.ink();
		assert(head.data);
		ptr head_p,the_p;
		head.seek(0),head.read(head_p);
		size_t pos = head_p.nxt_spa;
		size_t seg = pos / block_size;
		size_t off = pos % block_size;
		Block the = man.buf.get_block(ti.path,seg,true);
		the.ink();
		the.seek(off),the.read(the_p);
		if (the_p.nxt_spa == 0) the_p.nxt_spa = next_valid_pos(pos);
		size_t siz = rec.size();
		for (size_t i=0;i<siz;++i) embed(rec.at(i),ti.def.col_def.at(i).col_spec.len,the);
		// adjust nxt_spa
		head_p.nxt_spa = the_p.nxt_spa;
		the_p.nxt_spa = 0;
		// adjust tup
		the_p.pre_tup = 0;
		the_p.nxt_tup = head_p.nxt_tup;
		head_p.nxt_tup = pos;
		// adjust tup.nxt_tup
		if (the_p.nxt_tup != 0) {
			size_t nxt_seg = the_p.nxt_tup / block_size;
			size_t nxt_off = the_p.nxt_tup % block_size;
			Block nxt = man.buf.get_block(ti.path,nxt_seg,false);
			assert(nxt.data);
			nxt.ink();
			ptr nxt_p;
			nxt.seek(nxt_off),nxt.read(nxt_p);
			nxt_p.pre_tup = pos;
			nxt.seek(nxt_off),nxt.write(nxt_p);
			nxt.unpin();
		}
		head.seek(0),head.write(head_p);
		the.seek(off),the.write(the_p);
		head.unpin();
		the.unpin();
		return pos;
	}
	vector<Literal> Recorder::erase_record_at(size_t pt) {
		size_t seg = pt / block_size;
		size_t off = pt % block_size;
		Block head = man.buf.get_block(ti.path,0,false);
		Block the = man.buf.get_block(ti.path,seg,false);
		assert(head.data);
		assert(the.data);
		head.ink();
		the.ink();
		ptr head_p,the_p;
		the.seek(off),the.read(the_p);

		// deal return value
		size_t siz = ti.def.col_def.size();
		vector<Literal> ret(siz);
		for (size_t i=0;i<siz;++i) {
			ret.at(i).dtype = ti.def.col_def.at(i).col_spec.data_type;
			parse(ret.at(i),ti.def.col_def.at(i).col_spec.len,the);
		}

		// deal with pre
		size_t pre_seg = the_p.pre_tup / block_size;
		size_t pre_off = the_p.pre_tup % block_size;
		Block pre = man.buf.get_block(ti.path,pre_seg,false);
		assert(pre.data);
		pre.ink();
		ptr pre_p;
		pre.seek(pre_off),pre.read(pre_p);
		pre_p.nxt_tup = the_p.nxt_tup;
		pre.seek(pre_off),pre.write(pre_p);
		pre.unpin();

		// deal with nxt
		if (the_p.nxt_tup) {
			size_t nxt_seg = the_p.nxt_tup / block_size;
			size_t nxt_off = the_p.nxt_tup % block_size;
			Block nxt = man.buf.get_block(ti.path,nxt_seg,false);
			assert(nxt.data);
			nxt.ink();
			ptr nxt_p;
			nxt.seek(nxt_off),nxt.read(nxt_p);
			nxt_p.pre_tup = the_p.pre_tup;
			nxt.seek(nxt_off),nxt.write(nxt_p);
			nxt.unpin();
		}

		head.seek(0),head.read(head_p);
		the_p.pre_tup = the_p.nxt_tup = 0;
		the_p.nxt_spa = head_p.nxt_spa;
		head_p.nxt_spa = pt;

		head.seek(0),head.write(head_p);
		the.seek(off),the.write(the_p);
		the.raw_fill(0,in_len); // erase the payload
		head.unpin();
		the.unpin();

		return ret;
	}

	// implement class Manageri
	Manager::Manager() : cat(),tmp(*this),buf() {}
	Resulter Manager::new_resulter() {
		return Resulter(*this);
	}

}