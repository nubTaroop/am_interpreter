#include <iostream>
#include <sstream>
#include <string>
#include "am0_interpreter.hpp"

namespace am0_interpreter {
	//starts the machine
	//if logging is true the machine state will be printed out after every command
	bool am0::run(bool logging) {
		am0_func_visitor afv {*this};
		while (pc && (pc <= prog.size())) {
			if (logging) std::cout << *this << std::endl;
			//run the function at programm counter
			if (boost::apply_visitor(afv,prog[pc - 1])) continue;
			else return false;
		}
		if (pc) { std::cerr << "Program counter ran out of line" << std::endl; return false; }
		return true;
	}

	//sets the machine state to default
	void am0::reset() {
		pc = 1;
		d_stack.clear();
		mem.clear();
	}

	//print out a error message
	//if "line" is not empty, the postion of the last correct character read from "is" will be shown
	inline bool parse_error(std::istream& is, const std::string& line = "") {
		if (line != "") {
			int pos = is.tellg();
			for (int i = 0; i < pos; ++i) std::cerr << " ";
			std::cerr << "^" << std::endl;
		}
		std::cerr << "Error while parsing\n\n";
		is.clear();
		return false;
	}

	//parse code into the machine
	//file has to be true if "is" is a filestream in order to provide correct output
	bool am0::parse_prog(std::istream& is, bool file) {
		std::cout << "AM0 code:" << std::endl;
		int lnr = 0;
		std::string line;
		while (std::cout << std::to_string(++lnr) << ": " && std::getline(is,line)) {
			//Enable shebang, comment and new line support under UNIX like systems
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
			if (line == "" || line.substr(0,1) == "#") {
				--lnr;
				//move cursor back to start
				if (file) std::cout << "\x1b[0G";
				else std::cout << "\x1b[F";
				//remove last line
				std::cout << "\x1b[K";
				//print comments in blue and bold
				if (line.substr(0,2) != "#!") std::cout << "\x1b[34;1m" << line << "\x1b[m\n";
				continue;
			}
#endif
			if (file) std::cout << line << std::endl;
			std::stringstream ls {line};
			std::string keyword;
			int par;
			ls >> keyword;
			//read functions from "ls" and add them to program code container
			if (keyword == "ADD;") { prog.push_back(add); continue; }
			else if (keyword == "SUB;") { prog.push_back(sub); continue; }
			else if (keyword == "MUL;") { prog.push_back(mul); continue; }
			else if (keyword == "DIV;") { prog.push_back(div); continue; }
			else if (keyword == "MOD;") { prog.push_back(mod); continue; }
			else if (keyword == "LT;") { prog.push_back(lt); continue; }
			else if (keyword == "EQ;") { prog.push_back(eq); continue; }
			else if (keyword == "NE;") { prog.push_back(ne); continue; }
			else if (keyword == "GT;") { prog.push_back(gt); continue; }
			else if (keyword == "LE;") { prog.push_back(le); continue; }
			else if (keyword == "GE;") { prog.push_back(ge); continue; }
			else if (keyword == "LOAD") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(load,par)); continue; }}
			else if (keyword == "LIT") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(lit,par)); continue; }}
			else if (keyword == "STORE") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(store,par)); continue; }}
			else if (keyword == "JMP") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(jmp,par)); continue; }}
			else if (keyword == "JMC") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(jmc,par)); continue; }}
			else if (keyword == "READ") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(read,par)); continue; }}
			else if (keyword == "WRITE") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(write,par)); continue; }}
			return parse_error(is);
		}
		//Last code line number will be removed after input ends under UNIX like systems
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
		std::cout << "\x1b[0G\x1b[0K";
#endif
		std::cout << std::endl;
		is.clear();
		return true;
	}

	//parse a initial state into the machine
	//input syntax: (program counter, data stack, [memory])
	//multiple data stack elements are seperated by a colon
	//memory elements are read like: "address/value" and multiple elements are seperated by a comma
	//data stack is read in reverse order
	bool am0::parse_state(std::istream& is) {
		//temporary state container
		struct am0_state {
			unsigned int pc;
			std::vector<int> d_stack;
			std::map<int,int> mem;
		} state;
		std::cout << "AM0 state:" << std::endl;
		std::string line;
		std::getline(is,line);
		if (is.fail()) return parse_error(is);
		std::istringstream cs {line};
		if ( !((cs.get() == '(') && (cs >> state.pc) && (cs.get() == ',')) ) return parse_error(cs,line);
		if (!jmp_address_is_valid(state.pc)) return false;
		int value;
		std::vector<int> r_stack;
		if (cs.peek() != '-' && cs.peek() != ',') {
			if ( !(cs >> value) ) return parse_error(cs,line);
			else r_stack.push_back(value);
			while (cs.peek() != ',') {
				if ( !((cs.get() == ':') && (cs >> value)) ) return parse_error(cs,line);
				else r_stack.push_back(value);
			}
			for (auto rit = r_stack.rbegin(); rit != r_stack.rend(); ++rit) state.d_stack.push_back(*rit);
		}
		cs.ignore(2,',');
		int key;
		if (cs.get() != '[') return parse_error(cs,line);
		if (cs.peek() != ']') {
			if ( !((cs >> key) && (cs.get() == '/') && (cs >> value)) ) return parse_error(cs,line);
			else {
				if (state.mem.count(key)) return parse_error(cs,line);
				else state.mem[key] = value;
			}
			while (cs.peek() != ']') {
				if ( !((cs.get() == ',') && (cs >> key) && (cs.get() == '/') && (cs >> value)) )
					return parse_error(cs,line);
				else {
					if (state.mem.count(key)) return parse_error(cs,line);
					else state.mem[key] = value;
				}
			}
		}
		cs.ignore();
		if (cs.get() != ')') return parse_error(cs,line);
		//apply changes from temporary state container (the machine is always left in a working/default state)
		pc = state.pc;
		d_stack = state.d_stack;
		mem = state.mem;
		std::cout << std::endl;
		return true;
	}

	//prints out the state of the machine
	std::ostream& operator<<(std::ostream& os,const am0& o) {
		std::string ret = "(" + std::to_string(o.pc);
		//offset for the program counter (maximum character space needed is known at this point)
		for (size_t i=0; i < (std::to_string(o.prog.size()).length() - ret.length() + 1);++i) ret += ' ';
		ret += " , ";
		//print data stack in reverse order
		for (auto rit = o.d_stack.rbegin(); rit != o.d_stack.rend(); ++rit) ret += std::to_string(*rit) + ":";
		if (o.d_stack.size()) ret.pop_back();
		else ret += "-";
		ret += " , [";
		for (auto x : o.mem ) ret += std::to_string(x.first) + "/" + std::to_string(x.second) + ",";
		if (o.mem.size()) ret.pop_back();
		ret += "])";
		return os << ret;
	}

	//applicators to functions in the program code container{
	bool am0::am0_func_visitor::operator()(std::function<bool(am0&)>& f) {
		return f(this->am0_machine);
	}

	bool am0::am0_func_visitor::operator()(std::pair<std::function<bool(am0&,int)>, int>& fc) {
		return fc.first(this->am0_machine,fc.second);
	}

	bool am0::enough_arguments_on_stack(int amount) const {
		if (d_stack.size() < (size_t) amount) {
			std::cerr << "Not enough arguments on data stack\n\n";
			return false;
		}
		return true;
	}
	//}


	//check if "adr" is valid memory address
	bool am0::address_is_valid(int address, bool check_load) const {
		if (address < 0 || (check_load && !mem.count(address))) {
			std::cerr << "Invalid memory address\n\n";
			return false;
		}
		return true;
	}

	//check if "jmp_address" is a valid jump address
	//if "check_loop" is true the jump address can't be equal to the current program counter
	bool am0::jmp_address_is_valid(int jmp_address, bool check_loop) const {
		if (jmp_address < 0 || (size_t) jmp_address > prog.size()) {
			std::cerr << "Invalid jump address. Possible range [0-" + std::to_string(prog.size()) + "]\n\n";
			return false;
		}
		if (check_loop && (unsigned int) jmp_address == pc) {
			std::cerr << "Loop jump\n\n";
			return false;
		}
		return true;
	}

	//perform a binary operation on data stack
	bool am0::perform_bin_op(const std::function<void(int,int&)>& f) {
		if (!enough_arguments_on_stack(2)) return false;
		//use the function like: f(data stack@end,data stack@(end-1))
		//where f stores its result at data stack@(end-1)
		f(d_stack[d_stack.size() - 1], d_stack[d_stack.size() - 2]);
		//remove the used value at the end of data stack
		d_stack.pop_back();
		++pc;
		return true;
	}

	//operation: ADD
	bool am0::add(am0& a) {
		return a.perform_bin_op([] (int first, int& second) { second += first; });
	}

	//operation: SUB
	bool am0::sub(am0& a) {
		return a.perform_bin_op([] (int first, int& second) { second -= first; });
	}

	//operatiion: MUL
	bool am0::mul(am0& a) {
		return a.perform_bin_op([] (int first, int& second) { second *= first; });
	}

	//operation: DIV
	bool am0::div(am0& a) {
		if (a.d_stack.back()) { return a.perform_bin_op([] (int first, int& second) { second /= first; }); }
		else { std::cerr << "Null division\n\n"; return false; }
	}

	//operation: MOD
	bool am0::mod(am0& a) {
		if (a.d_stack.back()) { return a.perform_bin_op([] (int first, int& second) { second %= first; }); }
		else { std::cerr << "Null division\n\n"; return false;}
	}

	//operation: LT
	bool am0::lt(am0& a) {
		return a.perform_bin_op([] (int first, int& second) { second = second < first; });
	}

	//operation: EQ
	bool am0::eq(am0& a) {
		return a.perform_bin_op([] (int first, int& second) { second = second == first; });
	}

	//operation: NE
	bool am0::ne(am0& a) {
		return a.perform_bin_op([] (int first, int& second) { second = second != first; });
	}

	//operation: GT
	bool am0::gt(am0& a) {
		return a.perform_bin_op([] (int first, int& second) { second = second > first; });
	}

	//operation: LE
	bool am0::le(am0& a) {
		return a.perform_bin_op([] (int first, int& second) { second = second <= first; });
	}

	//operation: GE
	bool am0::ge(am0& a) {
		return a.perform_bin_op([] (int first, int& second) { second = second >= first; });
	}

	//operation: LOAD n
	bool am0::load(am0& a, int par) {
		if (!a.address_is_valid(par,true)) return false;
		//load at memory address from memory to data stack
		a.d_stack.push_back(a.mem[par]);
		++a.pc;
		return true;
	}

	//operation: LIT z
	bool am0::lit(am0& a, int par) {
		//add the literal to data stack
		a.d_stack.push_back(par);
		++a.pc;
		return true;
	}

	//operation: STORE n
	bool am0::store(am0& a, int par) {
		if (!a.address_is_valid(par) || !a.enough_arguments_on_stack(1)) return false;
		//store the value from data stack at memory address on memory
		a.mem[par] = a.d_stack.back();
		a.d_stack.pop_back();
		++a.pc;
		return true;
	}

	//operation: JMP e
	bool am0::jmp(am0& a, int par) {
		if (!a.jmp_address_is_valid(par, true)) return false;
		//change the program counter to jump address
		a.pc = par;
		return true;
	}

	//operation: JMC e
	bool am0::jmc(am0& a, int par) {
		if (!a.jmp_address_is_valid(par) || !a.enough_arguments_on_stack(1)) return false;
		//if the value at the end of the data stack is 0 pc will be changed to jump address
		if (a.d_stack.back() == 0) a.pc = par;
		//if the value at the end of the data stack is 1 pc will increased normaly
		else if (a.d_stack.back() == 1) ++a.pc;
		else { std::cerr << "Jump conditions have to be 1 or 0\n\n"; return false; }
		//remove the value at the end of the data stack
		a.d_stack.pop_back();
		return true;
	}

	//operation: READ n
	bool am0::read(am0& a, int par) {
		if (!a.address_is_valid(par)) return false;
		int i;
		std::cout << " In: "; std::cin >> i;
		//get value from stdin and store this value at memory address on memory
		if (std::cin.fail()) { std::cin.clear(); std::cerr << "Wrong input\n\n"; return false; }
		a.mem[par] = i;
		++a.pc;
		return true;
	}

	//operation: WRITE n
	bool am0::write(am0& a, int par) {
		if (!a.address_is_valid(par, true)) return false;
		//write value at memory address from memory to stdout
		std::cout << "Out: " << a.mem[par] << std::endl;
		++a.pc;
		return true;
	}
}
