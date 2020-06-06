/*
 * MeSQL - buffer - buffer.cpp
 * 
 * This file defines classes declared in buffer.hpp
 * 
 */

#include <string>
#include <sstream>
#include <numeric>
#include <cassert>
#include "../config.hpp"
#include "buffer.hpp"

namespace MeBuf {

	// implement class BlockSpec
	BlockSpec::BlockSpec(const string &_file_path,size_t _ord) : file_path(_file_path),ord(_ord) {
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
		vector<Node> &vec = g[key.hash_index];
		for (Node &v:vec) if (v.key == key) return v.val;
		vec.emplace_back(key);
		return vec.back().val;
	}

	// implement class BufferManager::Block
	BufferManager::Block::Block() : buf(NULL),inked(false),pinned(false),index(-1),it(),data(NULL) {};
	BufferManager::Block::Block(BufferManager *_buf,buffer_index_t _index,char *_data) : 
		buf(_buf),inked(false),pinned(true),index(_index),it(buf->uplist.end()),data(_data) {}
	void BufferManager::Block::ink() {
		inked = true;
	}
	void BufferManager::Block::pin() {
		if (pinned) return;
		buf->pin(*this);
	}
	void BufferManager::Block::unpin() {
		if (!pinned) return;
		buf->unpin(*this);
	}

	// implement class BufferManager::Pool
	BufferManager::Pool::Pool(BufferManager *_buf) : buf(_buf) {
		iota(pol,pol+block_num,0);
		cur = 0;
	}
	BufferManager::Pool::~Pool() {
		del_all();
	}
	BufferManager::Block BufferManager::Pool::new_block() { // assert not empty
		assert(!empty());
		buffer_index_t x = pol[cur++];
		buf->b[x] = BufferManager::Block(buf,x,&buf->m_data[x][0]);
		return buf->b[x];
	}
	void BufferManager::Pool::del_block(const Block &blo) {

	}
	void BufferManager::Pool::del_all() {
		while (cur--) {
			buffer_index_t x = pol[cur];
			del_block(buf->b[x]);
		}
	}
}