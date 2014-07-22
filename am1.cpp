#include <iostream>
#include <fstream>
#include <map>
#include "am1_interpreter.hpp"
#define __PROG_NAME__ "am1"

using namespace am1_interpreter;
using namespace std;

int main(int argc, char** argv) {
	bool logging = false;
	bool file = false;
	bool state = false;
	map<string,function<void()>> options = {
		{"-l", ([&] () {logging= true;})},
		{"--logging", ([&] () {logging= true;})},
		{"-i", ([&] () {state= true;})},
		{"--init", ([&] () {state= true;})}
	};
	if (argc == 2 && (string {"--help"} == argv[1])) {
		cout << "Call: am1 [OPTIONS] [INPUT-FILE]\nInterprets the INPUT-FILE as AM1-code.\n" <<
			"If no INPUT-FILE is given, user input will be interpreted.\n\n" <<
			"Options:\n" <<
			"  -l, --logging\t\tEnable AM1 state logging\n" <<
			"  -i, --init\t\tLet AM1 use a initial state\n\n" <<
			"End input of AM1-code with Ctrl+D\n";
		return 1;
	}
	am1 prog;
	enum {FILE_T = true};
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			if (options.count(argv[i])) options[argv[i]]();
			else {
				cerr << __PROG_NAME__ << ": Invalid option '" << argv[i] << "'" << endl <<
					"\"am1 --help\" gives further information." << endl;
				return 1;
			}
		}
		else {
			if (i == argc - 1) {
				ifstream fs { argv[i] };
				if (fs.fail()) {
					cerr << "Could not open file '" << argv[i] << "'" << endl;
					return 1;
				}
				if (!prog.parse_prog(fs, FILE_T)) return 1;
				file = true;
			}
			else {
				cerr << __PROG_NAME__ << ": Invalid argument '" << argv[i] << "'" << endl <<
					"\"am1 --help\" gives further information." << endl;
				return 1;
			}
		}
	}
	if (!file && !prog.parse_prog()) return 1;
	if (state) {
		bool once = true;
		while (!prog.parse_state()) {
			if (once) {
				cout << "Examples for valid AM1 states:\n" <<
					"(1,-,-,0) - default\n" <<
					"(14,2:3,2:4:15:0,4)\n" <<
					"(3,-,1:4:1,0)\n\n";
				once = false;
			}
		}
	}
	if (cout << "Running the AM1 interpreter:" << endl && !prog.run(logging)) {
		cerr << "AM1 interpreter terminated with an error.\nLast machine state: " << prog << endl;
	}
	else cout << "Final state: " << prog << endl;
	return 0;
}
