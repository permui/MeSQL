/*
 * MeSQL - buffer - buffer.hpp
 * 
 * This file declares class BufferManager and other related classes.
 *
 */

#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <string>
#include <list>
#include <vector>
#include "../config.hpp"

using namespace std;

namespace MeBuf {

	class BufferManager;

	typedef int buffer_index_t; // cannot be unsigned since -1 is used

	class BlockSpec {
	public:
		string file_path;
		size_t ord;
		size_t hash_index;

		BlockSpec(const string &_file_path,size_t _ord);
		size_t hash() const;
		bool operator== (const BlockSpec &b) const;
	};

	class HashTable { // map : BlockSpec -> index in buffer
	private:
		struct Node {
			BlockSpec key;
			buffer_index_t val; // initialize to -1

			Node(const BlockSpec &_key);
			Node(const BlockSpec &_key,buffer_index_t _val);
		};
		list<Node> g[hash_table_size];
		
	public:
		HashTable();
		bool have_key(const BlockSpec &key) const;
		buffer_index_t& operator[] (const BlockSpec &key);
		void erase(const BlockSpec &key);
	};

	class Block {
	private:
		BufferManager &buf;
		buffer_index_t index;
	public:
		char *data; // data null means not exists and not created

		Block(BufferManager &_buf,buffer_index_t _index,char *_data);
		void ink();
		void pin();
		void unpin();
	};

	class BufferManager {
	private:
		// unpinned block index list ; for LRU
		typedef list<buffer_index_t> List;
		typedef List::iterator itt;
		List uplist,uselist;
		itt list_node[block_num],use_node[block_num];
		// index pool
		buffer_index_t pol[block_num];
		buffer_index_t cur;
		
		HashTable h;

		char m_data[block_num][block_size],aux[block_size];

		class Info {
		public:
			BlockSpec bls;
			bool inked;
			bool pinned;
			Info();
			Info(const BlockSpec &_bls,bool _inked,bool _pinned);
		};
		Info info[block_num];
	public:
		BufferManager();
		~BufferManager();
		Block make_block(buffer_index_t index);
		Block empty_block();

		// read bls into to[] and return length read ; size(to) must >= block_size
		// throw an MeInternalError if file cannot be open 
		size_t read_block(const BlockSpec &bls,char *to);

		// write from to bls ; size(from) must >= block_size
		// throw an MeInternalError if file cannot be written
		void write_block(const BlockSpec &bls,char *from);

		// create if not exists means : if there is no this block **in the file** then create 
		// if not exists and not created , then empty_block = Block(*this,-1,nullptr) is returned
		// throw an MeInternalError if file cannot be read
		Block get_block(const string &_file_path,size_t _ord,bool create_if_not_exists);
		Block get_block(const BlockSpec &bls,bool create_if_not_exists);
		buffer_index_t new_index(const BlockSpec &bls);
		void flush_index(buffer_index_t index); // just write it to disk
		void del_index(buffer_index_t index); // flush and del, go back to pool
		void discard_one(); // choose appropriate block and del it ; throw InternalError if cannot
		void discard_all(); // del all block out of pool
		bool more() const;
		void ink(buffer_index_t index);
		void pin(buffer_index_t index);
		void unpin(buffer_index_t index);
	};

}

#endif