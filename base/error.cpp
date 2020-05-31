/*
 * MeSQL - base - error.cpp
 * 
 * This file implements function declares in error.hpp .
 *
 */

#include <iostream>
#include "error.hpp"

using namespace std;

namespace MeError {
	void MyError(const string &s) {
		cout << s << endl;
	}
}