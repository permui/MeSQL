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

	class TmpManager {
	private:
		size_t cnt;
		string tmp_name(size_t x) const;
	public:
		TmpManager();
		~TmpManager();
		string new_tmp();
		void ret_tmp(const string &);
	};

	// class Resulter mananges results from select / show statement
	class Resulter {
	private:
		Manager &man;
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

	class Manager {
	public:
		CatalogManager cat;
		TmpManager tmp;
		BufferManager buf;

		Manager(); 
	};

}

#endif