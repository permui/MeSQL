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
#include "../config.hpp"

static const char VERT = '|';
static const char HORI = '-';
static const char CORN = '+';

namespace MeMan {
	using namespace MeBuf;

	// implement class TmpManager
	TmpManager::TmpManager() : cnt(0) {}
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
		ofstream(ret); // create the file
		return ret;
	}
	void TmpManager::ret_tmp(const string &s) {
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
			TableColumnDef &def = col_defs[i];
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
			assert(tup[i].dtype == col_defs[i].col_spec.data_type);
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
			os << hor;
			os << VERT;
			for (size_t i=0;i<col_cnt;++i) {
				os << adjust(col_defs[i].col_name,col_width[i]);
				os << VERT;
			}
			os << hor;
			for (size_t i=0;i<row_cnt;++i) {
				os << VERT;
				for (size_t j=0;j<col_cnt;++j) {
					const Literal &lit = get_next_lit(col_defs[j].col_spec);
					os << adjust(lit.str_unquoted(),col_width[j]);
					os << VERT;
				}
				os << endl;
			}
			os << hor;
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
			os << hor;
			os << VERT;
			for (size_t i=0;i<_col_cnt;++i) {
				os << adjust(prt_spec[i].second,_col_width[i]);
				os << VERT;
			}
			os << hor;
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
			os << hor;
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
				memcpy(blo.data + reader_pos,&x,siz);
				ret = Literal(x);
				break;
			}
			case DataType::FLOAT : {
				float x;
				memcpy(blo.data + reader_pos,&x,siz);
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

	// implement class Manager
	Manager::Manager() : cat(),tmp(),buf() {}
	Resulter Manager::new_resulter() {
		return Resulter(*this);
	}

}