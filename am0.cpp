#include <iostream>
#include <fstream>
#include <map>
#include "am0_interpreter.hpp"
#define __PROG_NAME__ "am0"

using namespace am0_interpreter;
using namespace std;

int main(int argc, char** argv) {
	bool logging = false;
	bool file = false;
	bool state = false;
	//map parameters to options
	map<string,function<void()>> options = {
		//enable the state logging of the machine
		{"-l", ([&] () {logging= true;})},
		{"--logging", ([&] () {logging= true;})},
		//enable parsing of a inital state
		{"-i", ([&] () {state= true;})},
		{"--init", ([&] () {state= true;})}
	};
	if (argc == 2 && (string {"--help"} == argv[1])) {
		cout << "Call: am0 [OPTIONS] [INPUT-FILE]\nInterprets the INPUT-FILE as AM0-code.\n" <<
			"If no INPUT-FILE is given, user input will be interpreted.\n\n" <<
			"Options:\n" <<
			"  -l, --logging\t\tEnable AM0 state logging\n" <<
			"  -i, --init\t\tLet AM0 use a initial state\n\n" <<
			"End input of AM0-code with Ctrl+D\n";
		return 1;
	}
	am0 prog;
	enum {FILE_T = true};
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			//apply options
			if (options.count(argv[i])) options[argv[i]]();
			else {
				cerr << __PROG_NAME__ << ": Invalid option '" << argv[i] << "'" << endl <<
					"\"am0 --help\" gives further information." << endl;
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
				//parse from file
				if (!prog.parse_prog(fs, FILE_T)) return 1;
				file = true;
			}
			else {
				//parse from stdin
				cerr << __PROG_NAME__ << ": Invalid argument '" << argv[i] << "'" << endl <<
					"\"am0 --help\" gives further information." << endl;
				return 1;
			}
		}
	}
	//parse inital state if enabled
	if (!file && !prog.parse_prog()) return 1;
	if (state) {
		bool once = true;
		while (!prog.parse_state()) {
			if (once) {
				//if the user cant provide a correct machine state, a note how about the syntax will be shown once
				cout << "Examples for valid AM0 states:\n" <<
					"(1,-,[]) - default\n" <<
					"(14,2:3,[1/3,2/-1,3/2])\n" <<
					"(3,2,[1/0])\n\n";
				once = false;
			}
		}
	}
	//run the machine and show the final state at the end
	if (cout << "Running the AM0 interpreter:" << endl && !prog.run(logging)) {
		cerr << "AM0 interpreter terminated with an error.\nLast machine state: " << prog << endl;
	}
	else cout << "Final state: " << prog << endl;
	return 0;
}
