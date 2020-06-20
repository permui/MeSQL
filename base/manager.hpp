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
#include <cassert>
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
	public:
		class ptr {
		public:
			size_t pre_tup;
			size_t nxt_tup;
			size_t nxt_spa;

			ptr();
			ptr(size_t _pre_tup,size_t _nxt_tup,size_t _nxt_spa);
		};

		class RecNode {
		public:
			ptr p;
			vector<Literal> tup;
			
			RecNode();
		};

		// _ti should be from CatalogManager
		Recorder(Manager &_man,TableInfo &_ti);
		size_t next_valid_pos(size_t pos) const;
		void init_table();
		void remove_table();
		// return position in file
		RecNode header();
		RecNode get_recnode(size_t pt);
		vector<Literal> get_record(size_t pt);
		size_t place_record(vector<Literal> &rec);
		vector<Literal> erase_record_at(size_t pt);
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

	/* useage : 
	 *	init() -> start(true) -> finish()
	 *	       -> start(false)-> finish() -> dest()
	 */
	template<class T> class Lister {
	private:
		Manager &man;
	public:
		size_t num;
	private:
		size_t pos;
		size_t off;
		size_t seg;
		bool do_write;
		string path;
		Block blo;

		void adjust() {
			if (off + sizeof (T) > block_size) {
				++seg, off = 0;
				pos = seg * block_size + off;
				if (blo.data) blo.unpin(),blo.data = nullptr;
			}
			if (!blo.data) {
				blo = man.buf.get_block(path,seg,true);
				if (do_write) blo.ink();
			}
		}
	public:
		Lister(Manager &_man) :
			man(_man),num(0),pos(0),off(0),seg(0),do_write(false),path(),blo(man.buf,0,nullptr) {}
		void init() {
			num = 0, pos = 0, off = 0, seg = 0;
			do_write = false;
			path = man.tmp.new_tmp();
			blo.data = nullptr;
		}
		void start(bool _do_write) {
			do_write = _do_write;
			pos = 0, off = 0, seg = 0;
			assert(!blo.data);
		}
		void finish() {
			if (blo.data) blo.unpin(),blo.data = nullptr;
		}
		void dest() {
			assert(!blo.data);
			man.tmp.ret_tmp(path);
		}
		void push_back(const T &x) {
			adjust();
			blo.write(x);
			pos += sizeof(T);
			off += sizeof(T);
			++num;
		}
		T pop_front() {
			adjust();
			T ret;
			blo.read(ret);
			pos += sizeof(T);
			off += sizeof(T);
			return ret;
		}

	};

}

#endif