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
#include "base.hpp"

using namespace std;

namespace MeMan {

	using namespace MeCat;

	class TmpManager {
	private:
		size_t cnt;
	public:
		TmpManager();
		string new_tmp();
		void ret_tmp(const string &);
	};

	class Manager {
	public:
		CatalogManager cat;
		TmpManager tmp;

		Manager(); 
	};

}

#endif