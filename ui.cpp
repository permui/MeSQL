#include <iostream>
#include <sstream>
#include "interpreter/interpreter.hpp"
#include "base/manager.hpp"
using namespace std;
using namespace MeInt;
const static char prompt_str_1[] = ">>> ";
const static char prompt_str_2[] = "... ";
const static char bye_str[] = "Bye";
int main() {
	stringstream ss;
	cout << "MeSQL Shell - GCC " << __VERSION__;
	cout << " (" << __TIME__ << " " << __DATE__ << ")" << endl << endl;
	cout << prompt_str_1;
	MeMan::Manager *man = new MeMan::Manager;
	bool started = false;
	while (true) {
		char c = cin.get();
		if (cin.eof()) break;
		if (started || !isspace(c)) {
			started = true;
			ss << c;
			if (c==';') {
				Interpreter a(&ss,&cout,false,*man);
				a.clear();
				a.parse();
				started = false;
				ss.str(string());
				if (a.state == -1) break; // -1 state means got a "quit;" statement
			}
		}
		if (c=='\n') cout << (started ? prompt_str_2 : prompt_str_1);
	}
	cout << bye_str << endl;
	try {
		delete man;
	} catch (MeError::MeError &e) {
		cout << e.str() << endl;
	}
	return 0;
}