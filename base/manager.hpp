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

namespace MeMan {

	// forward declare ; TODO ; should be deleted when they are implemented.
	class CatalogManager;
	class BufferManager;

	class Manager {
	public:
		CatalogManager cat;
//		BufferManager buf;

		Manager(); 
	};

}

#endif