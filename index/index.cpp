/*
 * MeSQL - index - index.cpp
 * 
 * This file implements what is declared in index.hpp
 * 
 */

#include "../base/base.hpp"
#include "../buffer/buffer.hpp"
#include "../base/manager.hpp"
#include <cstring>
#include <cassert>
#include "index.hpp"

using namespace std;
using namespace MeType;
using namespace MeInfo;
using namespace MeBuf;

static Literal read_literal_from_block(Block &blo,DataType dtype,char_size_t len) {
	Literal ret;
	ret.dtype = dtype;
	switch (dtype) {
		case DataType::INT : 
			blo.read(ret.int_val);
			break;
		case DataType::FLOAT :
			blo.read(ret.float_val);
			break;
		case DataType::CHAR : {
			static char aux[block_size],*r;
			blo.raw_read(aux,len);
			for (r=aux+len;r!=aux && *(r-1)==0;--r);
			ret.char_val.assign(aux,r);
			break;
		}
	}
	return ret;
}

static void write_literal_to_block(Block &blo,char_size_t len,const Literal &lit) {
	switch (lit.dtype) {
		case DataType::INT :
			blo.write(lit.int_val);
			break;
		case DataType::FLOAT :
			blo.write(lit.float_val);
			break;
		case DataType::CHAR : {
			size_t my = lit.char_val.length();
			assert(my <= len);
			blo.raw_write(lit.char_val.c_str(),my);
			blo.raw_fill(0,len - my);
			break;
		}
	}
}

namespace MeInd {

	// implement class BNode
	BNode::BNode(size_t _seg) : seg(_seg),is_leaf(false),P(),K() {}

	// implement class Noter::empter
	Noter::empter::empter(size_t _seg) : seg(_seg),nxt_spa(0),root(0) {}

	// implement class Noter
	Noter::empter Noter::get_empter(size_t seg) {
		Block blo = man.buf.get_block(di.path,seg,true);
		empter ret(seg);
		blo.seek(0);
		blo.read(ret.nxt_spa);
		blo.read(ret.root);
		blo.unpin();
		return ret;
	}
	void Noter::write_empter(const empter &e) {
		Block blo = man.buf.get_block(di.path,e.seg,true);
		blo.ink();
		blo.seek(0);
		blo.raw_fill(0,block_size);
		blo.seek(0);
		blo.write(e.nxt_spa);
		blo.write(e.root);
		blo.unpin();
	}

	Noter::Noter(Manager &_man,const IndexInfo &_di) : 
		man(_man),di(_di),ptr_size(sizeof (size_t)),key_size(di.col.col_spec.len) {
		N = (block_size + key_size - 1 - ptr_size) / (key_size + ptr_size);
	}
	BNode Noter::get_bnode(size_t seg) {
		Block blo = man.buf.get_block(di.path,seg,false);
		assert(blo.data);
		BNode ret(seg);
		blo.seek(0);

		size_t m;
		blo.read(m);
		assert(m > 0);
		ret.P.resize(m),ret.K.resize(m-1);

		blo.read(ret.is_leaf);

		DataType dtype = di.col.col_spec.data_type;
		for (size_t i=0;i<m;++i) blo.read(ret.P.at(i));
		for (size_t i=0;i<m-1;++i) ret.K.at(i) = read_literal_from_block(blo,dtype,di.col.col_spec.len);
		blo.unpin();
		return ret;
	}
	void Noter::write_bnode(const BNode &b) {
		Block blo = man.buf.get_block(di.path,b.seg,true);
		blo.ink();
		blo.seek(0);
		blo.write(b.P.size());
		blo.write(b.is_leaf);
		for (const size_t &i:b.P) blo.write(i);
		for (const Literal &lit:b.K) write_literal_to_block(blo,di.col.col_spec.len,lit);
		blo.unpin();
	}
	BNode Noter::new_bnode() {
		empter head = get_empter(0);
		size_t seg = head.nxt_spa;
		empter the(seg);
		Block blo = man.buf.get_block(di.path,head.nxt_spa,true);
		blo.ink();
		blo.seek(0),blo.read(the.nxt_spa);
		head.nxt_spa = the.nxt_spa ? the.nxt_spa : (the.seg + 1);
		blo.seek(0),blo.raw_fill(0,block_size);
		blo.seek(0);
		blo.write(static_cast<size_t>(1));
		blo.write(false);
		blo.unpin();
		write_empter(head);
		return get_bnode(seg);
	}
	void Noter::del_bnode(const BNode &b) {
		size_t seg = b.seg;
		empter head = get_empter(0);
		empter the(seg);
		the.nxt_spa = head.nxt_spa;
		head.nxt_spa = seg;
		write_empter(head);
		write_empter(the);
	}
	size_t Noter::get_root() {
		empter head = get_empter(0);
		return head.root;
	}
	void Noter::change_root(size_t rt) {
		empter head = get_empter(0);
		head.root = rt;
		write_empter(head);
	}

	// implement class Tree
	typedef pair<Literal,size_t> ins_rev;

	Tree::Tree(Noter &_noter) : noter(_noter) {}
	ins_rev Tree::_insert(size_t x,const Literal &val,size_t pos) {
		assert(x);
		BNode now = noter.get_bnode(x);
		if (now.is_leaf) {
			size_t j = upper_bound(now.K.begin(),now.K.end(),val) - now.K.begin();
			now.K.insert(now.K.begin() + j,val);
			now.P.insert(now.P.begin() + j,pos);
			return split_leaf(now);
		} else {
			size_t j = upper_bound(now.K.begin(),now.K.end(),val) - now.K.begin();
			ins_rev res = _insert(now.P.at(j),val,pos);
			if (res.second) {
				now.K.insert(now.K.begin() + j,res.first);
				now.P.insert(now.P.begin() + j + 1,res.second);
				return split_inner(now); // write node inside
			}
			return ins_rev(Literal(),0);
		}
	}
	ins_rev Tree::split_leaf(BNode &now) {
		assert(now.is_leaf);
		size_t N = noter.N;
		if (now.P.size() <= N) {
			noter.write_bnode(now);
			return ins_rev(Literal(),0);
		}
		assert(now.P.size() == N + 1);
		BNode nxt = noter.new_bnode();
		nxt.is_leaf = now.is_leaf;
		size_t rig = N / 2;
		size_t lef = N - rig;
		nxt.P.resize(rig + 1),nxt.K.resize(rig);
		copy(now.P.begin() + lef,now.P.end(),nxt.P.begin());
		copy(now.K.begin() + lef,now.K.end(),nxt.K.begin());
		now.P.erase(now.P.begin() + lef,now.P.end());
		now.K.erase(now.K.begin() + lef,now.K.end());
		now.P.push_back(nxt.seg);
		ins_rev ret(nxt.K.front(),nxt.seg);
		noter.write_bnode(now);
		noter.write_bnode(nxt);
		return ret;
	}
	ins_rev Tree::split_inner(BNode &now) {
		size_t N = noter.N;
		if (now.P.size() <= N) {
			noter.write_bnode(now);
			return ins_rev(Literal(),0);
		}
		assert(now.P.size() == N + 1);
		BNode nxt = noter.new_bnode();
		nxt.is_leaf = now.is_leaf;
		size_t rig = (N + 1) / 2;
		size_t lef = (N + 1) - rig;
		nxt.P.resize(rig),nxt.K.resize(rig - 1);
		copy(now.P.begin() + lef,now.P.end(),nxt.P.begin());
		copy(now.K.begin() + lef,now.K.end(),nxt.K.begin());
		ins_rev ret(now.K.at(lef-1),nxt.seg);
		now.P.erase(now.P.begin() + lef,now.P.end());
		now.K.erase(now.K.begin() + lef - 1,now.K.end());
		noter.write_bnode(now);
		noter.write_bnode(nxt);
		return ret;
	}
	void Tree::insert(const Literal &val,size_t pos) {
		size_t root = noter.get_root();
		ins_rev res = _insert(root,val,pos);
		if (res.second != 0) {
			BNode nw = noter.new_bnode();
			nw.is_leaf = false;
			nw.P.clear(),nw.K.clear();
			nw.P.push_back(root),nw.P.push_back(res.second);
			nw.K.push_back(res.first);
			noter.change_root(nw.seg);
			noter.write_bnode(nw);
		}
	}

	typedef pair<Literal,bool> era_rev;

	void Tree::adjust_two(BNode &now,size_t fir,size_t sec) { // write son
		assert(fir + 1 == sec);
		BNode one = noter.get_bnode(fir);
		BNode two = noter.get_bnode(sec);
		if (one.is_leaf) {
			assert(one.P.size() >= 1);
			one.P.pop_back();
			for (size_t v:two.P) one.P.push_back(v);
			for (const Literal &v:two.K) one.K.push_back(v);
			size_t all = one.K.size();
			if (all < noter.N) {
				now.K.erase(now.K.begin() + fir);
				now.P.erase(now.P.begin() + sec);
				noter.del_bnode(two);
				noter.write_bnode(one);
				return;
			} else {
				size_t rig = all / 2;
				size_t lef = all - rig;
				two.K.resize(rig);
				two.P.resize(rig + 1);
				copy(one.K.begin() + lef,one.K.end(),two.K.begin());
				copy(one.P.begin() + lef,one.P.end(),two.P.begin());
				now.K.at(fir) = two.K.front();
				one.P.erase(one.P.begin() + lef,one.P.end());
				one.K.erase(one.K.begin() + lef,one.K.end());
				one.P.push_back(two.seg);
				noter.write_bnode(one);
				noter.write_bnode(two);
				return;
			}
		} else {
			one.K.push_back(now.K.at(fir));
			for (size_t v:two.P) one.P.push_back(v);
			for (const Literal &v:two.K) one.K.push_back(v);
			size_t all = one.P.size();
			if (all <= noter.N) {
				now.K.erase(now.K.begin() + fir);
				now.P.erase(now.P.begin() + sec);
				noter.del_bnode(two);
				noter.write_bnode(one);
				return;
			} else {
				size_t rig = all / 2;
				size_t lef = all - rig;
				two.K.resize(rig - 1);
				two.P.resize(rig);
				copy(one.K.begin() + lef,one.K.end(),two.K.begin());
				copy(one.P.begin() + lef,one.P.end(),two.P.begin());
				now.K.at(fir) = one.K.at(lef-1);
				one.P.erase(one.P.begin() + lef,one.P.end());
				one.K.erase(one.K.begin() + lef - 1,one.K.end());
				noter.write_bnode(one);
				noter.write_bnode(two);
				return;
			}
		}
	}
	void Tree::adjust(BNode &now,size_t j) { // write now
		assert(now.P.size() >= 2);
		assert(j < now.P.size());
		if (j + 1 == now.P.size()) adjust_two(now,j-1,j);
		else adjust_two(now,j,j+1);
		noter.write_bnode(now);
	}
	era_rev Tree::_erase(size_t x,const Literal &val,size_t pos) {
		BNode now = noter.get_bnode(x);
		if (now.is_leaf) {
			size_t j = lower_bound(now.K.begin(),now.K.end(),val) - now.K.begin();
			assert(j < now.K.size() && now.K.at(j) == val && now.P.at(j) == pos);
			now.K.erase(now.K.begin() + j);
			now.P.erase(now.P.begin() + j);
			Literal ol = now.K.empty() ? Literal() : now.K.front();
			bool ob = now.K.size() < noter.N / 2;
			noter.write_bnode(now);
			return era_rev(ol,ob);
		} else {
			size_t j = upper_bound(now.K.begin(),now.K.end(),val) - now.K.begin();
			era_rev res = _erase(now.P.at(j),val,pos);
			pair<Literal,bool> out(res.first,false);
			bool need_write = false;
			if (!res.first.null() && j > 0) {
				now.K.at(j-1) = res.first;
				out.first = Literal();
				need_write = true;
			}
			if (res.second) {
				adjust(now,j); // write inside
				out.second = (now.P.size() < (noter.N + 1) / 2);
			} else if (need_write) noter.write_bnode(now);
			return out;
		}
	}
	void Tree::erase(const Literal &val,size_t pos) {
		size_t root = noter.get_root();
		era_rev res = _erase(root,val,pos);
		if (res.second) {
			BNode now = noter.get_bnode(root);
			if (now.is_leaf) return;
			if (now.P.size() >= 2) return;
			assert(now.P.size() == 1);
			size_t nr = now.P.front();
			noter.del_bnode(now);
			noter.change_root(nr);
		}
	}
	BNode Tree::_lefest_leaf(size_t x) {
		assert(x);
		BNode now = noter.get_bnode(x);
		return now.is_leaf ? now : _lefest_leaf(now.P.front());
	}
	pair<BNode,size_t> Tree::_first_greater_pos(size_t x,const Literal &val) {
		BNode now = noter.get_bnode(x);
		if (now.is_leaf) {
			size_t j = upper_bound(now.K.begin(),now.K.end(),val) - now.K.begin();
			if (j < now.K.size()) return pair<BNode,size_t>(now,j);
			size_t y = now.P.back();
			if (y == 0) return pair<BNode,size_t>(BNode(0),0);
			BNode nxt = noter.get_bnode(y);
			assert(val < nxt.K.front());
			return pair<BNode,size_t>(nxt,0);
		} else {
			size_t j = upper_bound(now.K.begin(),now.K.end(),val) - now.K.begin();
			return _first_greater_pos(now.P.at(j),val);
		}
	}
	pair<BNode,size_t> Tree::_first_greater_equal_pos(size_t x,const Literal &val) {
		BNode now = noter.get_bnode(x);
		if (now.is_leaf) {
			size_t j = lower_bound(now.K.begin(),now.K.end(),val) - now.K.begin();
			if (j < now.K.size()) return pair<BNode,size_t>(now,j);
			size_t y = now.P.back();
			if (y == 0) return pair<BNode,size_t>(BNode(0),0);
			BNode nxt = noter.get_bnode(y);
			assert(val < nxt.K.front());
			return pair<BNode,size_t>(nxt,0);
		} else {
			size_t j = upper_bound(now.K.begin(),now.K.end(),val) - now.K.begin();
			return _first_greater_equal_pos(now.P.at(j),val);
		}
	}
	BNode Tree::lefest_leaf() {
		size_t root = noter.get_root();
		return _lefest_leaf(root);
	}
	pair<BNode,size_t> Tree::first_greater_pos(const Literal &val) {
		size_t root = noter.get_root();
		return _first_greater_pos(root,val);
	}
	pair<BNode,size_t> Tree::first_greater_equal_pos(const Literal &val) {
		size_t root = noter.get_root();
		return _first_greater_equal_pos(root,val);
	}


	// implement class Indexer
	Indexer::Indexer(Manager &_man,IndexInfo &_di) : man(_man),di(_di),noter(man,di),tree(noter) {}
	void Indexer::insert_record(const vector<Literal> &tup,size_t pos) {
		const Literal &the = tup.at(di.def.col_ord);
		tree.insert(the,pos);
	}
	void Indexer::delete_record(const vector<Literal> &tup,size_t pos) {
		const Literal &the = tup.at(di.def.col_ord);
		tree.erase(the,pos);
	}
	pair<BNode,size_t> Indexer::first_leaf_start() {
		BNode b = tree.lefest_leaf();
		return {b,0};
	}
	pair<BNode,size_t> Indexer::first_greater_pos(const Literal &lit) {
		return tree.first_greater_pos(lit);
	}
	pair<BNode,size_t> Indexer::first_greater_equal_pos(const Literal &lit) {
		return tree.first_greater_equal_pos(lit);
	} 
	BNode Indexer::get_bnode(size_t seg) {
		return noter.get_bnode(seg);
	}
	void Indexer::init_index() {
		di.path = INDEX_DIR + di.def.index_name + INDEX_SUF;
		Block head = man.buf.get_block(di.path,0,true);
		head.ink();
		head.write(static_cast<size_t>(1));
		head.write(static_cast<size_t>(1));
		head.unpin();
		BNode rt = noter.new_bnode();
		assert(rt.seg == 1);
		rt.is_leaf = true;
		rt.P.push_back(0);
		noter.write_bnode(rt);
	}
	void Indexer::remove_index() {
		man.buf.remove_file(di.path);
		remove(di.path.c_str());
	}
}