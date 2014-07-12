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
	map<string,function<void()>> options = {
		{"-l", ([&] () {logging= true;})},
		{"--logging", ([&] () {logging= true;})},
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
				if (!am0::parse(prog, fs, FILE_T)) return 1;
				file = true;
			}
			else {
				cerr << __PROG_NAME__ << ": Invalid argument '" << argv[i] << "'" << endl <<
					"\"am0 --help\" gives further information." << endl;
				return 1;
			}
		}
	}
	if (!file && !am0::parse(prog)) return 1;
	if (state) {
		am0_state_t am0_state;
		bool once = true;
		while (!am0::parse(am0_state)) {
			if (once) {
				cout << "Examples for valid AM0 states:\n" <<
					"{1,-,[]} (default)\n" <<
					"{14,2:3,[1/3,2/-1,3/2]}\n" <<
					"{3,2,[1/0]}\n\n";
				once = false;
			}
		}
		if (!prog.set_state(am0_state)) return 1;
	}
	if (cout << "Running the AM0 interpreter:" << endl && !prog.run(logging)) {
		cerr << "AM0 interpreter terminated with an error.\nLast machine state: " << prog << endl;
	}
	return 0;
}
