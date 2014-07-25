#include <sstream>
#include "am1_interpreter.hpp"

namespace am1_interpreter {
	//starts the machine
	//if logging is true the machine state will be printed out after every command
	bool am1::run(bool logging) {
		am1_func_visitor afv {*this};
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
	void am1::reset(void) {
		pc = 1;
		d_stack.clear();
		rt_stack.clear();
		ref = 0;
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
	bool am1::parse_prog(std::istream& is, bool file) {
		std::cout << "AM1 code:" << std::endl;
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
			if (keyword == "ADD;") { prog.push_back(am0::add); continue; }
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
			else if (keyword == "PUSH;") { prog.push_back(push); continue; }
			else if (keyword == "LIT") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(lit,par)); continue; }}
			else if (keyword == "JMP") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(jmp,par)); continue; }}
			else if (keyword == "JMC") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(jmc,par)); continue; }}
			else if (keyword == "CALL") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(call,par)); continue; }}
			else if (keyword == "INIT") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(init,par)); continue; }}
			else if (keyword == "RET") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(ret,par)); continue; }}
			else {
				ls.seekg(0);
				ls >> std::ws;
				if ( !std::getline(ls,keyword,'(') ) return parse_error(is);
				if (keyword == "LOADI") { if (ls >> par && ls.get() == ')') {
					prog.push_back(std::make_pair(loadi,par)); continue;}}
				else if (keyword == "STOREI") { if (ls >> par && ls.get() == ')') {
					prog.push_back(std::make_pair(storei,par)); continue;}}
				else if (keyword == "READI") { if (ls >> par && ls.get() == ')') {
					prog.push_back(std::make_pair(readi,par)); continue;}}
				else if (keyword == "WRITEI") { if (ls >> par && ls.get() == ')') {
					prog.push_back(std::make_pair(writei,par)); continue;}}
				else {
					std::string visible;
					if ( !std::getline(ls,visible,',') ) return parse_error(is);
					visibility v;
					if (visible == "global") v = global;
					else if (visible == "local" || visible == "lokal") v = local;
					else return parse_error(is);
					if (keyword == "LOAD") {
						if (ls >> par && ls.get() == ')' && ls.get() == ';') {
							prog.push_back(std::make_tuple(load,v,par)); continue;}}
					else if (keyword == "STORE") {
						if (ls >> par && ls.get() == ')' && ls.get() == ';') {
							prog.push_back(std::make_tuple(store,v,par)); continue;}}
					else if (keyword == "READ") {
						if (ls >> par && ls.get() == ')' && ls.get() == ';') {
							prog.push_back(std::make_tuple(read,v,par)); continue;}}
					else if (keyword == "WRITE") {
						if (ls >> par && ls.get() == ')' && ls.get() == ';') {
							prog.push_back(std::make_tuple(write,v,par)); continue;}}
					else if (keyword == "LOADA") {
						if (ls >> par && ls.get() == ')' && ls.get() == ';') {
							prog.push_back(std::make_tuple(loada,v,par)); continue;}}
				}
			}
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

	//check if "ra" is valid return address
	bool am1::ra_address_is_valid(int ra) const {
		if (ra <= 0 || (size_t) ra > prog.size()) {
			std::cerr << "Invalid return address. Possible range [1-" + std::to_string(prog.size()) + "]\n\n";
			return false;
		}
		return true;
	}

	//check if "adr" is valid memory address
	bool am1::address_is_valid(visibility s, int adr) const {
		if ((s == global && (adr <= 0 || (size_t) adr > rt_stack.size())) ||
			(s == local && ((adr + (int) ref) <= 0 || (size_t) ((int) ref + adr) > rt_stack.size()))) {
				std::cerr << "Invalid memory address\n\n";
				return false;
		}
		return true;
	}

	//check if "jmp_address" is a valid jump address
	//if "check_loop" is true the jump address can't be equal to the current program counter
	bool am1::jmp_address_is_valid(int jmp_address, bool check_loop) const {
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

	//parse a initial state into the machine
	//input syntax: (program counter, data stack, runtime stack, ref)
	//multiple data or runtime stack elements are seperated by a colon
	//data stack is read in reverse order
	bool am1::parse_state(std::istream& is) {
		//temporary state container
		struct am0_state {
			unsigned int pc;
			std::vector<int> d_stack;
			std::vector<int> rt_stack;
			unsigned int ref;
		} state;
		std::cout << "AM1 state:" << std::endl;
		std::string line;
		std::getline(is,line);
		if (is.fail()) return parse_error(is);
		std::istringstream cs {line};
		if ( !((cs.get() == '(') && (cs >> state.pc) && (cs.get() == ',')) ) return parse_error(cs,line);
		if (!ra_address_is_valid(state.pc)) return false;
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
		if (cs.peek() != '-' && cs.peek() != ',') {
			if ( !(cs >> value) ) return parse_error(cs,line);
			else state.rt_stack.push_back(value);
			while (cs.peek() != ',') {
				if ( !((cs.get() == ':') && (cs >> value)) ) return parse_error(cs,line);
				else state.rt_stack.push_back(value);
			}
		}
		cs.ignore(2,',');
		if ((cs >> state.ref) && cs.get() != ')') return parse_error(cs,line);
		//apply changes from temporary state container (the machine is always left in a working/default state)
		pc = state.pc;
		d_stack = state.d_stack;
		rt_stack = state.rt_stack;
		ref = state.ref;
		std::cout << std::endl;
		return true;
	}

	//prints out the state of the machine
	std::ostream& operator<<(std::ostream& os, const am1& o) {
		std::string ret = "(" + std::to_string(o.pc);
		//offset for the program counter (maximum character space needed is known at this point)
		for (size_t i=0; i < (std::to_string(o.prog.size()).length() - ret.length() + 1);++i) ret += ' ';
		ret += " , ";
		//print data stack in reverse order
		for (auto rit = o.d_stack.rbegin(); rit != o.d_stack.rend(); ++rit) ret += std::to_string(*rit) + ":";
		if (o.d_stack.size()) ret.pop_back();
		else ret += "-";
		ret += " , ";
		for (auto x : o.rt_stack ) ret += std::to_string(x) + ":";
		if (o.rt_stack.size()) ret.pop_back();
		else ret += "-";
		ret += " , " + std::to_string(o.ref) + ")";
		return os << ret;
	}

	//applicators to functions in the program code container{
	bool am1::am1_func_visitor::operator()(std::function<bool(am0&)>& f) {
		return f(this->am1_machine);
	}

	bool am1::am1_func_visitor::operator()(std::pair<std::function<bool(am0&,int)>, int>& fc) {
		return fc.first(this->am1_machine,fc.second);
	}

	bool am1::am1_func_visitor::operator()(std::function<bool(am1&)>& f) {
		return f(this->am1_machine);
	}

	bool am1::am1_func_visitor::operator()(std::pair<std::function<bool(am1&,int)>,int>& fc) {
		return fc.first(this->am1_machine,fc.second);
	}

	bool am1::am1_func_visitor::operator()(std::tuple<std::function<bool(am1&,visibility,int)>,visibility,int>& fc) {
		return (std::get<0>(fc))(this->am1_machine,std::get<1>(fc),std::get<2>(fc));
	}
	//}

	//operation: LOAD(b,o)
	bool am1::load(am1& a, visibility s, int adr) {
		if (!a.address_is_valid(s,adr)) return false;
		//load value at memory address from runtime stack to data stack
		a.d_stack.push_back(a.rt_stack[adr - 1 + ((s == local) ? a.ref : 0)]);
		a.pc++;
		return true;
	}

	//operation: STORE(b,o)
	bool am1::store(am1& a, visibility s, int adr) {
		if (!a.enough_arguments_on_stack(1) || !a.address_is_valid(s,adr)) return false;
		//store value from data stack at memory address on runtime stack
		a.rt_stack[adr - 1 + ((s == local) ? a.ref : 0)] = a.d_stack.back();
		//remove value from data stack
		a.d_stack.pop_back();
		a.pc++;
		return true;
	}

	//operation: READ(b,o)
	bool am1::read(am1& a, visibility s, int adr) {
		if (!a.address_is_valid(s,adr)) return false;
		int i;
		//get value from stdin and store this value at memory address on runtime stack
		std::cout << " In: "; std::cin >> i;
		if (std::cin.fail()) { std::cin.clear(); std::cerr << "Wrong input\n\n"; return false; }
		a.rt_stack[adr - 1 + ((s == local) ? a.ref : 0)] = i;
		a.pc++;
		return true;
	}

	//operation: WRITE(b,o)
	bool am1::write(am1& a, visibility s, int adr) {
		if (!a.address_is_valid(s,adr)) return false;
		//write value at memory address from runtime stack to stdout
		std::cout << "Out: " << a.rt_stack[adr - 1 + ((s == local) ? a.ref : 0)] << std::endl;
		a.pc++;
		return true;
	}

	//operation: LOADI(o)
	bool am1::loadi(am1& a, int adr) {
		if (!a.address_is_valid(local,adr)) return false;
		//dereference address and call LOAD(global,*o)
		return am1::load(a,global,a.rt_stack[adr - 1 + a.ref]);
	}

	//operation: STOREI(o)
	bool am1::storei(am1& a, int adr) {
		if (!a.address_is_valid(local,adr)) return false;
		//dereference address and call STORE(global,*o)
		return am1::store(a,global,a.rt_stack[adr - 1 + a.ref]);
	}

	//operation: READI(o)
	bool am1::readi(am1& a, int adr) {
		if (!a.address_is_valid(local,adr)) return false;
		//dereference address and call READ(global,*o)
		return am1::read(a,global,a.rt_stack[adr - 1 + a.ref]);
	}

	//operation: WRITEI(o)
	bool am1::writei(am1& a, int adr) {
		if (!a.address_is_valid(local,adr)) return false;
		//dereference address and call WRITE(global,*o)
		return am1::write(a,global,a.rt_stack[adr - 1 + a.ref]);
	}

	//operation: LOADA(b,o)
	bool am1::loada(am1& a, visibility s, int adr) {
		if (!a.address_is_valid(s,adr)) return false;
		//load relative address to data stack
		a.d_stack.push_back(adr + ((s == local) ? a.ref : 0));
		a.pc++;
		return true;
	}

	//operation: PUSH
	bool am1::push(am1& a) {
		if (!a.enough_arguments_on_stack(1)) return false;
		//store value from data stack at the end of the runtime stack
		a.rt_stack.push_back(a.d_stack.back());
		//remove value from data stack
		a.d_stack.pop_back();
		a.pc++;
		return true;
	}

	//operation: CALL adr
	bool am1::call(am1& a, int adr) {
		if (!a.ra_address_is_valid(adr)) return false;
		//store return address at the end of the runtime stack
		a.rt_stack.push_back(a.pc + 1);
		//store previous activation record at the end of the runtime stack
		a.rt_stack.push_back(a.ref);
		//update program counter
		a.pc = adr;
		//update ref to new point of the last previous activation record
		a.ref = a.rt_stack.size();
		return true;
	}

	//operation: INIT n
	bool am1::init(am1& a, int par) {
		if (par < 0) {
			std::cerr << "Init only takes arguments >= 0\n\n";
			return false;
		}
		//equal to n-times:
		//LIT 0;
		//PUSH;
		for (int i = 0; i < par; ++i) a.rt_stack.push_back(0);
		a.pc++;
		return true;
	}

	//operation: RET n
	bool am1::ret(am1& a, int par) {
		if (par < 0 || a.rt_stack.size() < (size_t) (par + 2)) {
			std::cerr << "Not enough values on runtime stack\n\n";
			return false;
		}
		if (a.ref > a.rt_stack.size()) {
			std::cerr << "Invalid ref. Not enough values on runtime stack\n\n";
			return false;
		}
		if (!a.ra_address_is_valid(a.rt_stack[a.ref - 2]) ||
			a.rt_stack[a.ref - 1] > ((int) a.ref - 2) || a.rt_stack[a.ref - 1] < 0) {
			std::cerr << "Can't return. Invalid arguments on runtime stack\n\n";
			return false;
		}
		//return old program counter
		a.pc = a.rt_stack[a.ref - 2];
		//save a temporary backup of the old ref
		unsigned int oldref = a.ref;
		//return previous activation record to ref
		a.ref = a.rt_stack[a.ref - 1];
		//remove localy initialised values from runtime stack
		while (a.rt_stack.size() > oldref) a.rt_stack.pop_back();
		//remove n local parameters, the return address and the previous activation record from runtime stack
		for (int i = 0; i < (par + 2); ++i) a.rt_stack.pop_back();
		return true;
	}
}
