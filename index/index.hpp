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
		size_t seg;
		bool is_leaf;
		vector<size_t> P;
		vector<Literal> K;

		BNode(size_t _seg);
	};

	// Noter for BNode operation
	class Noter {
	private:
		IndexInfo &di;

	};

}

#endif