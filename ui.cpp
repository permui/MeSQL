#include <iostream>
#include <sstream>
#include "interpreter/interpreter.hpp"
using namespace std;
using namespace MeInt;
const static char prompt_str_1[] = ">>> ";
const static char prompt_str_2[] = "... ";
const static char bye_str[] = "Bye";
int main() {
	stringstream ss;
	Interpreter a(&ss,&cout,false);
	cout << "MeSQL Shell" << endl << endl;
	cout << prompt_str_1;
	bool started = false;
	while (true) {
		char c = cin.get();
		if (cin.eof()) {
			cout << bye_str << endl;
			break;
		}
		if (started || !isspace(c)) {
			started = true;
			ss << c;
			if (c==';') {
				a.clear();
				a.parse();
				started = false;
				ss.str(string());
			}
		}
		if (c=='\n') cout << (started ? prompt_str_2 : prompt_str_1);
	}
	return 0;
}