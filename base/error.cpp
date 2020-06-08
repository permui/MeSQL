/*
 * MeSQL - base - error.cpp
 * 
 * This file implements classes and functions declares in error.hpp .
 *
 */

#include <iostream>
#include <string>
#include "error.hpp"

using namespace std;

namespace MeError {

	void MyError(const string &s) {
		cout << s << endl;
	}

	// implement class MeError
	MeError::MeError() : pre(),msg("Unknown Error") {}
	MeError::MeError(const string &_msg) : pre("Error"),msg(_msg) {}
	MeError::MeError(const string &_pre,const string &_msg) : pre(_pre),msg(_msg) {}
	string MeError::str() const {
		return pre.empty() ? msg : (pre + " : " + msg);
	}

}