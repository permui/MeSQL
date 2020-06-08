/*
 * MeSQL - buffer - buffer.cpp
 * 
 * This file defines classes declared in buffer.hpp
 * 
 */

#include <string>
#include <sstream>
#include <fstream>
#include <numeric>
#include <cassert>
#include <cstring>
#include <algorithm>
#include "../config.hpp"
#include "buffer.hpp"
#include "../base/error.hpp"

namespace MeBuf {

	// implement class BlockSpec
	BlockSpec::BlockSpec(const string &_file_path,size_t _ord) : file_path(_file_path),ord(_ord) {
		reverse(file_path.begin(),file_path.end()); // for fast difference detection for performance
		assert(ord < max_block_ord); // so further operation would not cause overflow
		const pair<size_t,size_t> pr = hash();
		file_hash = pr.first % hash_table_size;
		hash_index = pr.second % hash_table_size;
	}
	bool BlockSpec::operator== (const BlockSpec &b) const {
		return file_path == b.file_path && ord == b.ord;
	}
	pair<size_t,size_t> BlockSpec::hash() const { // djb2 hash function ; not moded
		static stringstream ss;
		ss.str("");
		size_t ret1 = 5381;
		for (char c:file_path) ret1 = ret1 * 33 + c;
		size_t ret2 = ret1 * 33 + ':';
		ss << ord;
		for (char c:ss.str()) ret2 = ret2 * 33 + c;
		return {ret1,ret2};
	}

	// implement class HashTable
	HashTable::Node::Node(const BlockSpec &_key,buffer_index_t _val) : key(_key),val(_val) {}
	HashTable::HashTable() {}
	bool HashTable::have_key(const BlockSpec &key) const {
		for (const Node &v:g[key.hash_index]) if (v.key == key) return true;
		return false;
	}
	buffer_index_t HashTable::at(const BlockSpec &key) const {
		for (const Node &v:g[key.hash_index]) if (v.key == key) return v.val;
		return -1;
	}
	buffer_index_t& HashTable::operator[] (const BlockSpec &key) {
		list<Node> &lis = g[key.hash_index];
		for (Node &v:lis) if (v.key == key) return v.val;
		lis.emplace_front(key,-1);
		return lis.front().val;
	}
	void HashTable::erase(const BlockSpec &key) {
		bool flag = false;
		list<Node> &lis = g[key.hash_index];
		for (list<Node>::iterator it=lis.begin();it!=lis.end();++it) if (it->key == key) {
			lis.erase(it);
			flag = true;
			break;
		}
		if (!flag) throw MeError::MeInternalError("hash table erase key not exists");
	}
	void HashTable::insert(const BlockSpec &key,buffer_index_t val) {
		g[key.hash_index].emplace_front(key,val);
	}

	// implement class FileTable
	FileTable::Node::Node(const string &_key) : key(_key),val() {}
	FileTable::FileTable() {}
	bool FileTable::have_key(const BlockSpec &bs) const {
		for (const Node &x:g[bs.file_hash]) if (x.key == bs.file_path) return true;
		return false;
	}
	List& FileTable::operator[] (const BlockSpec &bs) {
		for (Node &x:g[bs.file_hash]) if (x.key == bs.file_path) return x.val;
		g[bs.file_hash].emplace_front(bs.file_path);
		return g[bs.file_hash].front().val;
	}
	void FileTable::erase(const string &key) {
		BlockSpec bs(key,0);
		erase(bs);
	}
	void FileTable::erase(const BlockSpec &bs) {
		list<Node> &lis = g[bs.file_hash];
		bool flag = false;
		for (list<Node>::iterator it=lis.begin();it!=lis.end();++it) if (it->key == bs.file_path) {
			lis.erase(it);
			flag = true;
			break;
		}
		if (!flag) throw MeError::MeInternalError("file table erase key not exists");
	}

	// implement class Block
	Block::Block(BufferManager &_buf,buffer_index_t _index,char *_data) : 
		buf(_buf),index(_index),data(_data) {}
	void Block::ink() {
		buf.ink(index);
	}
	void Block::unpin() {
		buf.unpin(index);
	}

	// implement class BufferManager::Info
	BufferManager::Info::Info() : bls("",0),inked(false),pinned(false) {}
	BufferManager::Info::Info(const BlockSpec &_bls,bool _inked,bool _pinned) : 
		bls(_bls),inked(_inked),pinned(_pinned) {}

	// implement class BufferManager
	BufferManager::BufferManager() : uplist(),uselist(),cur(0),h(),f() {
		fill(begin(list_node),end(list_node),uplist.end());
		fill(begin(use_node),end(use_node),uselist.end());
		iota(begin(pol),end(pol),0);
	}
	BufferManager::~BufferManager() {
		discard_all();
	}
	Block BufferManager::make_block(buffer_index_t index) {
		return Block(*this,index,m_data[index]);
	}
	Block BufferManager::empty_block() {
		return Block(*this,-1,nullptr);
	}
	size_t BufferManager::read_block(const BlockSpec &bls,char *to) {
		ifstream f(bls.file_path);
		if (f.fail()) throw MeError::MeInternalError("cannot open db file '" + bls.file_path + "'");
		memset(to,0,block_size);
		f.seekg(block_size * bls.ord,f.beg);
		f.read(to,block_size);
		return f.gcount();
	}
	void BufferManager::write_block(const BlockSpec &bls,char *from) {
		ofstream f(bls.file_path);
		if (f.fail()) throw MeError::MeInternalError("cannot open db file '" + bls.file_path + "'");
		f.seekp(block_size * bls.ord,f.beg);
		f.write(from,block_size);
		if (f.fail()) throw MeError::MeInternalError("fail write db file '" + bls.file_path + "'");
	}
	Block BufferManager::get_block(const string &_file_path,size_t _ord,bool create_if_not_exists) {
		BlockSpec bls(_file_path,_ord);
		return get_block(bls,create_if_not_exists);
	}
	Block BufferManager::get_block(const BlockSpec &bls,bool create_if_not_exists) {
		buffer_index_t idx = h.at(bls);
		if (idx != -1) {
			pin(idx);
			return make_block(idx);
		}
		size_t len = read_block(bls,aux);
		if (len || create_if_not_exists) {
			buffer_index_t index = new_index(bls);
			h.insert(bls,index); // assume by logic not exists
			List &lis = f[bls];
			fp[index] = lis.insert(lis.end(),index);
			memcpy(m_data[index],aux,block_size);
			Block ret = make_block(index);
			if (len==0) ret.ink();
			return ret;
		}
		return empty_block();
	}
	buffer_index_t BufferManager::new_index(const BlockSpec &bls) {
		if (!more()) discard_one();
		assert(more());
		buffer_index_t ret = pol[cur++];
		list_node[ret] = uplist.end();
		use_node[ret] = uselist.insert(uselist.end(),ret);
		info[ret] = Info(bls,false,true);
		return ret;
	}
	void BufferManager::flush_index(buffer_index_t index) {
		if (!info[index].inked) return;
		write_block(info[index].bls,m_data[index]);
		info[index].inked = false;
	}
	void BufferManager::del_index(buffer_index_t index,bool flush) {
		if (flush) flush_index(index);
		if (list_node[index] != uplist.end()) {
			uplist.erase(list_node[index]);
			list_node[index] = uplist.end();
		}
		assert(use_node[index] != uselist.end());
		uselist.erase(use_node[index]);
		use_node[index] = uselist.end();
		--cur,pol[cur] = index;
		const BlockSpec &bls = info[index].bls;
		h.erase(bls);
		List &lis = f[bls];
		lis.erase(fp[index]);
		if (lis.empty()) f.erase(bls);
		info[index] = Info();
	}
	void BufferManager::discard_one() {
		if (uplist.empty()) throw MeError::MeInternalError(
			"cannot discard one in buffer, because there is no unpinned block"
		);
		del_index(uplist.front(),true);
	}
	void BufferManager::discard_all() {
		vector<buffer_index_t> vec(uselist.begin(),uselist.end()); // since uselist is changing during del_index
		for (buffer_index_t x:vec) del_index(x,true);
	}
	void BufferManager::remove_file(const string &file_path) {
		BlockSpec bls(file_path,0);
		if (!f.have_key(bls)) return;
		List lis = f[bls];
		assert(!lis.empty());
		for (buffer_index_t idx:lis) del_index(idx,false);
		assert(!f.have_key(bls));
	}
	bool BufferManager::more() const {
		return cur < block_num;
	}
	void BufferManager::ink(buffer_index_t index) {
		info[index].inked = true;
	}
	void BufferManager::pin(buffer_index_t index) {
		if (info[index].pinned) return;
		info[index].pinned = true;
		assert(list_node[index] != uplist.end());
		uplist.erase(list_node[index]);
		list_node[index] = uplist.end();
	}
	void BufferManager::unpin(buffer_index_t index) {
		if (!info[index].pinned) return;
		info[index].pinned = false;
		assert(list_node[index] == uplist.end());
		list_node[index] = uplist.insert(uplist.end(),index);
	}
}