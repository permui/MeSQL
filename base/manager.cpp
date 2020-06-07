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
	TmpManager::~TmpManager() {
		for (size_t i=1;i<=cnt;++i) remove(tmp_name(i).c_str());
	}
	string TmpManager::tmp_name(size_t x) const {
		stringstream ss;
		ss << DB_FILES "tmp/" << x << ".tmp";
		return ss.str();
	}
	string TmpManager::new_tmp() {
		string ret = tmp_name(++cnt);
		ofstream(ret); // create the file
		return ret;
	}
	void TmpManager::ret_tmp(const string &s) {
		remove(s.c_str()); // delete the file
	}

	// implement class Manager
	Manager::Manager() : cat(),tmp(),buf() {}

}