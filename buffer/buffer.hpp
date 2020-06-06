/*
 * MeSQL - buffer - buffer.hpp
 * 
 * This file declares class BufferManager and others.
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

	class BlockSpec;
	class HashTable;
	class BufferManager;

	typedef int buffer_index_t;

	class BlockSpec {
	public:
		string file_path;
		size_t ord;
		size_t hash_index;

		BlockSpec(const string &_file_path,size_t _ord);
		size_t hash() const;
		bool operator== (const BlockSpec &b) const;
	};

	class HashTable {
	private:
		struct Node {
			BlockSpec key;
			buffer_index_t val; // initialize to -1

			Node(const BlockSpec &_key);
			Node(const BlockSpec &_key,buffer_index_t _val);
		};
		vector<Node> g[hash_table_size];
		
	public:
		HashTable();
		bool have_key(const BlockSpec &key) const;
		buffer_index_t& operator[] (const BlockSpec &key);
	};

	class BufferManager {
	private:
		// unpinned block's index list ; used for LRU
		typedef list<buffer_index_t> List;
		List uplist;
		
		char m_data[block_num][block_size];

	public:
		class Block {
		private:
			BufferManager *buf;
			bool inked;
			bool pinned;
		public:
			buffer_index_t index; // *this 's index in b[]
			List::iterator it; // initialize to uplist.end()
			char *data;

			Block();
			Block(BufferManager *_buf,buffer_index_t _index,char *_data);
			void ink();
			void pin();
			void unpin();
		};
		Block b[block_num];

	private:
		class Pool {
		private:
			BufferManager *buf;
			buffer_index_t pol[block_num];
			buffer_index_t cur;
		public:
			Pool(BufferManager *_buf);
			~Pool();
			Block new_block();
			void del_block(const Block &blo);
			void del_all();
			bool empty() const;
		};
		Pool pool;

		HashTable h;
		BlockSpec where[block_num];

	public:
		BufferManager();
		~BufferManager();
		Block want(const string &_file_name,size_t ord,bool create_if_not_exists);
		void pin(Block &blo);
		void unpin(Block &blo);
	private:
		void discard();

		friend class Pool;
	};

	typedef BufferManager::Block Block;

}

#endif