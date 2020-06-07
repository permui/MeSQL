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
	MeError::MeError() : msg("Unknown Error") {}
	MeError::MeError(const string &_msg) : msg(_msg) {}
	string MeError::str() const {
		return "Error : " + msg;
	}

	// implement class MeInternalError
	string MeInternalError::str() const {
		return "InternalError : " + msg;
	}
}