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