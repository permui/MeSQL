/*
 * MeSQL - base - manager.hpp
 * 
 * This header declares the class Manager, which aggregates 
 * all the managers used when executing statements.
 * Those managers are declared and defined elsewhere.
 * 
 */

#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <string>
#include "../catalog/catalog.hpp"
#include "../buffer/buffer.hpp"
#include "base.hpp"

using namespace std;

namespace MeMan {

	using namespace MeCat;
	using namespace MeBuf;
	using namespace MeInfo;
	class Manager;

	class TmpManager {
	private:
		Manager &man;
		size_t cnt;
		string tmp_name(size_t x) const;
	public:
		TmpManager(Manager &_man);
		~TmpManager();
		string new_tmp();
		void ret_tmp(const string &);
	};

	// class Resulter mananges results from select / show statement
	// how to use : init -> add_tuple -> print -> finish
	class Resulter {
	private:
		Manager &man;

		// what matter in TableColumnDef are :
		// def.col_name, def.col_spec.data_type, def.col_spec.len
		vector<TableColumnDef> col_defs;
		vector<int> col_width;
		size_t col_cnt;
		size_t row_cnt; // number of tuples in result
		string file;
		size_t writer_block,writer_pos; // pos in block
		size_t reader_block,reader_pos;

		string horizontal_line(size_t col_cnt,const vector<int> &col_width) const;
		void write_item(const Literal &lit,size_t siz);
		string adjust(string s,int width); // _ (s__) _ ; ( width )
		Literal get_next_lit(const TableColumnSpec &spec);
	public:
		Resulter(Manager &_man);
		void init(const vector<TableColumnDef> &c);
		void finish(); // must be called when used
		void add_tuple(const vector<Literal> &tup);
		void print(ostream &os); // either type of print only once for a Resulter
		// if provided name is empty, then original name of provided col is used
		void print(ostream &os,vector<pair<size_t,string>> prt_spec);
	};

	class Recorder {
	private:
		Manager &man;
		TableInfo &ti;
		size_t in_len;
		size_t out_len;
		class ptr {
		public:
			size_t pre_tup;
			size_t nxt_tup;
			size_t nxt_spa;

			ptr();
			ptr(size_t _pre_tup,size_t _nxt_tup,size_t _nxt_spa);
		};
	public:
		// _ti should be from CatalogManager
		Recorder(Manager &_man,TableInfo &_ti);
		size_t next_valid_pos(size_t pos) const;
		void init_table();
		// return position in file
		vector<Literal> get_record(size_t pt);
		size_t place_record(const vector<Literal> &rec);
		void erase_record_at(size_t pt);
		// they will be specialized
		// just write / read, without any check
		template<typename T> void embed(const T &val,size_t len,Block &blo);
		template<typename T> void parse(T &val,size_t len,Block &blo);
	};

	class Manager {
	public:
		CatalogManager cat;
		TmpManager tmp;
		BufferManager buf;

		Manager(); 
		Resulter new_resulter();
	};

}

#endif