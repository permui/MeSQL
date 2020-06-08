/*
 * MeSQL - base - error.hpp
 * 
 * This file declares classes and functions for error handle.
 *
 */

#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>

using namespace std;

namespace MeError {
	
	void MyError(const string &s);

	class MeError {
	protected:
		string pre;
		string msg;
	public:
		MeError();
		MeError(const string &_msg);
		MeError(const string &_pre,const string &_msg);
		string str() const;
	};

}

#endif