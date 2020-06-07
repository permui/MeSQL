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
#include "../config.hpp"
#include "buffer.hpp"
#include "../base/error.hpp"

namespace MeBuf {

	// implement class BlockSpec
	BlockSpec::BlockSpec(const string &_file_path,size_t _ord) : file_path(_file_path),ord(_ord) {
		assert(ord < max_block_ord); // so further operation would not cause overflow
		hash_index = hash() % hash_table_size;
	}
	bool BlockSpec::operator== (const BlockSpec &b) const {
		return file_path == b.file_path && ord == b.ord;
	}
	size_t BlockSpec::hash() const { // djb2 hash function ; not moded
		stringstream ss;
		ss << file_path << ":" << ord;
		size_t ret = 5381;
		for (char c:ss.str()) ret = ret * 33 + c;
		return ret;
	}

	// implement class HashTable
	HashTable::Node::Node(const BlockSpec &_key) : key(_key),val(-1) {}
	HashTable::Node::Node(const BlockSpec &_key,buffer_index_t _val) : key(_key),val(_val) {}
	HashTable::HashTable() {}
	bool HashTable::have_key(const BlockSpec &key) const {
		for (const Node &v:g[key.hash_index]) if (v.key == key) return true;
	}
	buffer_index_t& HashTable::operator[] (const BlockSpec &key) {
		list<Node> &lis = g[key.hash_index];
		for (Node &v:lis) if (v.key == key) return v.val;
		lis.emplace_back(key);
		return lis.back().val;
	}
	void HashTable::erase(const BlockSpec &key) {
		bool flag = false;
		list<Node> &lis = g[key.hash_index];
		for (list<Node>::iterator it=lis.begin();it!=lis.end();++it) if (it->key == key) {
			lis.erase(it);
			flag = true;
			break;
		}
		assert(flag);
		if (!flag) throw MeError::MeInternalError("hash table erase key not exists");
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
	BufferManager::BufferManager() : uplist(),uselist(),cur(0),h() {
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
		if (h.have_key(bls)) {
			buffer_index_t index = h[bls];
			pin(index);
			return make_block(index);
		}
		size_t len = read_block(bls,aux);
		if (len || create_if_not_exists) {
			buffer_index_t index = new_index(bls);
			h[bls] = index;
			memcpy(m_data[index],aux,block_size);
			return make_block(index);
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
	void BufferManager::del_index(buffer_index_t index) {
		flush_index(index);
		if (list_node[index] != uplist.end()) {
			uplist.erase(list_node[index]);
			list_node[index] = uplist.end();
		}
		assert(use_node[index] != uselist.end());
		uselist.erase(use_node[index]);
		use_node[index] = uselist.end();
		--cur,pol[cur] = index;
		h.erase(info[index].bls);
		info[index] = Info();
	}
	void BufferManager::discard_one() {
		if (uplist.empty()) throw MeError::MeInternalError(
			"cannot discard one in buffer, because there is no unpinned block"
		);
		del_index(uplist.front());
	}
	void BufferManager::discard_all() {
		vector<buffer_index_t> vec(uselist.begin(),uselist.end()); // since uselist is changing during del_index
		for (buffer_index_t x:vec) del_index(x);
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