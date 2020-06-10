/*
 * MeSQL - index - index.hpp
 * 
 * This file declares several class for index manipulation.
 * The final API is provided by class Indexer.
 * It relies on class Tree for low-level B+ Tree operation.
 * 
 */

#ifndef INDEX_HPP
#define INDEX_HPP

#include "../base/manager.hpp"
#include "../base/base.hpp"

using namespace std;

namespace MeInd {

	using namespace MeMan;
	using namespace MeInfo;
	
	// B+ Tree node in memory state
	class BNode {
	public:
		size_t seg; // seg == 0 means null node
		bool is_leaf;
		vector<size_t> P;
		vector<Literal> K;

		BNode(size_t _seg);
	};

	// Noter for BNode operation
	class Noter {
	private:
		Manager &man;
		const IndexInfo &di;
		size_t ptr_size;
		size_t key_size;
		size_t node_cnt;

		class empter {
		public:
			size_t seg;
			size_t nxt_spa;
			size_t root;

			empter(size_t _seg);
		};
		empter get_empter(size_t seg);
		// it will smash the whole block and write
		void write_empter(const empter &e);
	public:
		size_t N;
		Noter(Manager &_man,const IndexInfo &_di);
		BNode get_bnode(size_t seg);
		void write_bnode(const BNode &b);
		BNode new_bnode();
		void del_bnode(const BNode &b);
		size_t get_root();
		void change_root(size_t rt);
	};

	class Tree {
	private:
		Noter &noter;
		BNode _lefest_leaf(size_t x);
		pair<BNode,size_t> _first_greater_pos(size_t x,const Literal &val);
		pair<BNode,size_t> _first_greater_equal_pos(size_t x,const Literal &val);
		pair<Literal,size_t> _insert(size_t x,const Literal &val,size_t pos);
		pair<Literal,size_t> split_inner(BNode &now);
		pair<Literal,size_t> split_leaf(BNode &now);
		pair<Literal,bool> _erase(size_t x,const Literal &val,size_t pos);
		void adjust_two(BNode &now,size_t fir,size_t sec);
		void adjust(BNode &now,size_t j);
	public:
		Tree(Noter &_noter);
		void insert(const Literal &val,size_t pos);
		void erase(const Literal &val,size_t pos);
		BNode lefest_leaf();
		pair<BNode,size_t> first_greater_pos(const Literal &val);
		pair<BNode,size_t> first_greater_equal_pos(const Literal &val);
	};
	
	class Indexer {
	private:
		Manager &man;
		IndexInfo &di;
		Noter noter;
		Tree tree;
	public:
		// init by IndexInfo
		Indexer(Manager &_man,IndexInfo &_di);

		// insert tup, its position in file is pos
		// just choose the key among tup
		void insert_record(const vector<Literal> &tup,size_t pos);
		void delete_record(const vector<Literal> &tup,size_t pos);
		pair<BNode,size_t> first_leaf_start();
		pair<BNode,size_t> first_greater_pos(const Literal &val);
		pair<BNode,size_t> first_greater_equal_pos(const Literal &val);
		BNode get_bnode(size_t seg);
		void init_index();
		void remove_index();
	};

}

#endif