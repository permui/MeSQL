/*
 * MeSQL - base - manager.cpp
 * 
 * This file defines the class declared in manager.hpp . 
 * 
 */

#include <sstream>
#include <fstream>
#include <string>
#include <cstdio>
#include "manager.hpp"
#include "../config.hpp"

namespace MeMan {

	// implement class TmpManager
	TmpManager::TmpManager() : cnt(0) {}
	string TmpManager::new_tmp() {
		++cnt;
		stringstream ss;
		ss << DB_FILES "tmp/" << cnt << ".tmp";
		string ret = ss.str();
		ofstream(ret); // create the file
		return ret;
	}
	void TmpManager::ret_tmp(const string &s) {
		remove(s.c_str()); // delete the file
	}

	// implement class Manager
	Manager::Manager() : cat(),tmp() {}

}