#include <sstream>
#include "am1_interpreter.hpp"

namespace am1_interpreter {
	bool am1::run(bool logging) {
		am1_func_visitor afv {*this};
		while (pc && (pc <= prog.size())) {
			if (logging) std::cout << *this << std::endl;
			if (boost::apply_visitor(afv,prog[pc - 1])) continue;
			else return false;
		}
		if (pc) { std::cerr << "Program counter ran out of line" << std::endl; return false; }
		return true;
	}

	void am1::reset(void) {
		pc = 1;
		d_stack.clear();
		rt_stack.clear();
		ref = 0;
	}

	inline bool parse_error(std::istream& is, const std::string& line = "", int offset = 0) {
		if (line != "") {
			int pos = is.tellg();
			for (int i = 0; i < pos + offset; ++i) std::cerr << " ";
			std::cerr << "^" << std::endl;
		}
		std::cerr << "Error while parsing\n\n";
		is.clear();
		return false;
	}

	bool am1::parse_prog(std::istream& is, bool file) {
		std::cout << "AM1 code:" << std::endl;
		int lnr = 0;
		std::string line;
		while (std::cout << std::to_string(++lnr) << ": " && std::getline(is,line)) {
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
			if (line == "" || line.substr(0,1) == "#") {
				--lnr;
				if (file) std::cout << "\x1b[0G";
				else std::cout << "\x1b[F";
				std::cout << "\x1b[K";
				if (line.substr(0,2) != "#!") std::cout << "\x1b[34;1m" << line << "\x1b[m\n";
				continue;
			}
#endif
			if (file) std::cout << line << std::endl;
			std::stringstream ls {line};
			std::string keyword;
			int par;
			ls >> keyword;
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
				prog.push_back(std::make_pair(lit,par)); continue; }
			}
			else if (keyword == "JMP") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(jmp,par)); continue; }
			}
			else if (keyword == "JMC") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(jmc,par)); continue; }
			}
			else if (keyword == "CALL") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(call,par)); continue; }
			}
			else if (keyword == "INIT") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(init,par)); continue; }
			}
			else if (keyword == "RET") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				prog.push_back(std::make_pair(ret,par)); continue; }
			}
			else {
				ls.seekg(0);
				ls >> std::ws;
				std::string id;
				if ( !std::getline(ls,id,'(') ) return parse_error(is);
				if (id == "LOADI") { if (ls >> par && ls.get() == ')') {
					prog.push_back(std::make_pair(loadi,par)); continue;}
				}
				else if (id == "STOREI") { if (ls >> par && ls.get() == ')') {
					prog.push_back(std::make_pair(storei,par)); continue;}
				}
				else if (id == "READI") { if (ls >> par && ls.get() == ')') {
					prog.push_back(std::make_pair(readi,par)); continue;}
				}
				else if (id == "WRITEI") { if (ls >> par && ls.get() == ')') {
					prog.push_back(std::make_pair(writei,par)); continue;}
				}
				else if (id == "LOAD") { if ( std::getline(ls,id,',') ) {
					if (id == "global" && ls >> par && ls.get() == ')' && ls.get() == ';') {
						prog.push_back(std::make_tuple(load,global,par)); continue;}
					else if (id == "local" && ls >> par && ls.get() == ')' && ls.get() == ';') {
						prog.push_back(std::make_tuple(load,local,par)); continue;}}
				}
				else if (id == "STORE") { if ( std::getline(ls,id,',') ) {
					if (id == "global" && ls >> par && ls.get() == ')' && ls.get() == ';') {
						prog.push_back(std::make_tuple(store,global,par)); continue;}
					else if (id == "local" && ls >> par && ls.get() == ')' && ls.get() == ';') {
						prog.push_back(std::make_tuple(store,local,par)); continue;}}
				}
				else if (id == "READ") { if ( std::getline(ls,id,',') ) {
					if (id == "global" && ls >> par && ls.get() == ')' && ls.get() == ';') {
						prog.push_back(std::make_tuple(read,global,par)); continue;}
					else if (id == "local" && ls >> par && ls.get() == ')' && ls.get() == ';') {
						prog.push_back(std::make_tuple(read,local,par)); continue;}}
				}
				else if (id == "WRITE") { if ( std::getline(ls,id,',') ) {
					if (id == "global" && ls >> par && ls.get() == ')' && ls.get() == ';') {
						prog.push_back(std::make_tuple(write,global,par)); continue;}
					else if (id == "local" && ls >> par && ls.get() == ')' && ls.get() == ';') {
						prog.push_back(std::make_tuple(write,local,par)); continue;}}
				}
				else if (id == "LOADA") { if ( std::getline(ls,id,',') ) {
					if (id == "global" && ls >> par && ls.get() == ')' && ls.get() == ';') {
						prog.push_back(std::make_tuple(loada,global,par)); continue;}
					else if (id == "local" && ls >> par && ls.get() == ')' && ls.get() == ';') {
						prog.push_back(std::make_tuple(loada,local,par)); continue;}}
				}
				return parse_error(ls,id);
			};
		}
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
		std::cout << "\x1b[0G\x1b[0K";
#endif
		std::cout << std::endl;
		is.clear();
		return true;
	}

	bool am1::ra_address_is_valid(int ra) const {
		if (ra <= 0 || (size_t) ra > prog.size()) {
			std::cerr << "Invalid return address. Possible range [1-" + std::to_string(prog.size()) + "]\n\n";
			return false;
		}
		return true;
	}

	bool am1::address_is_valid(state s, int adr) const {
		if ((s == global && (adr <= 0 || (size_t) adr > rt_stack.size())) ||
			(s == local && ((adr + (int) ref) <= 0 || (size_t) ((int) ref + adr) > rt_stack.size()))) {
				std::cerr << "Invalid memory address\n\n";
				return false;
		}
		return true;
	}

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

	bool am1::parse_state(std::istream& is) {
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
		pc = state.pc;
		d_stack = state.d_stack;
		rt_stack = state.rt_stack;
		ref = state.ref;
		std::cout << std::endl;
		return true;
	}

	std::ostream& operator<<(std::ostream& os, const am1& o) {
		std::string ret = "(" + std::to_string(o.pc);
		for (size_t i=0; i < (std::to_string(o.prog.size()).length() - ret.length() + 1);++i) ret += ' ';
		ret += " , ";
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

	bool am1::am1_func_visitor::operator()(std::tuple<std::function<bool(am1&,state,int)>,state,int>& fc) {
		return (std::get<0>(fc))(this->am1_machine,std::get<1>(fc),std::get<2>(fc));
	}

	bool am1::load(am1& a, state s, int adr) {
		if (!a.address_is_valid(s,adr)) return false;
		a.d_stack.push_back(a.rt_stack[adr - 1 + ((s == local) ? a.ref : 0)]);
		a.pc++;
		return true;
	}

	bool am1::store(am1& a, state s, int adr) {
		if (!a.enough_arguments_on_stack(1) || !a.address_is_valid(s,adr)) return false;
		a.rt_stack[adr - 1 + ((s == local) ? a.ref : 0)] = a.d_stack.back();
		a.d_stack.pop_back();
		a.pc++;
		return true;
	}

	bool am1::read(am1& a, state s, int adr) {
		if (!a.address_is_valid(s,adr)) return false;
		int i;
		std::cout << " In: "; std::cin >> i;
		if (std::cin.fail()) { std::cin.clear(); std::cerr << "Wrong input\n\n"; return false; }
		a.rt_stack[adr - 1 + ((s == local) ? a.ref : 0)] = i;
		a.pc++;
		return true;
	}

	bool am1::write(am1& a, state s, int adr) {
		if (!a.address_is_valid(s,adr)) return false;
		std::cout << "Out: " << a.rt_stack[adr - 1 + ((s == local) ? a.ref : 0)] << std::endl;
		a.pc++;
		return true;
	}

	bool am1::loadi(am1& a, int adr) {
		if (!a.address_is_valid(local,adr)) return false;
		return am1::load(a,global,a.rt_stack[adr - 1 + a.ref]);
	}

	bool am1::storei(am1& a, int adr) {
		if (!a.address_is_valid(local,adr)) return false;
		return am1::store(a,global,a.rt_stack[adr - 1 + a.ref]);
	}

	bool am1::readi(am1& a, int adr) {
		if (!a.address_is_valid(local,adr)) return false;
		return am1::read(a,global,a.rt_stack[adr - 1 + a.ref]);
	}

	bool am1::writei(am1& a, int adr) {
		if (!a.address_is_valid(local,adr)) return false;
		return am1::write(a,global,a.rt_stack[adr - 1 + a.ref]);
	}

	bool am1::loada(am1& a, state s, int adr) {
		if (!a.address_is_valid(s,adr)) return false;
		a.d_stack.push_back(adr + ((s == local) ? a.ref : 0));
		a.pc++;
		return true;
	}

	bool am1::push(am1& a) {
		if (!a.enough_arguments_on_stack(1)) return false;
		a.rt_stack.push_back(a.d_stack.back());
		a.d_stack.pop_back();
		a.pc++;
		return true;
	}

	bool am1::call(am1& a, int adr) {
		if (!a.ra_address_is_valid(adr)) return false;
		a.rt_stack.push_back(a.pc + 1);
		a.rt_stack.push_back(a.ref);
		a.pc = adr;
		a.ref = a.rt_stack.size();
		return true;
	}

	bool am1::init(am1& a, int par) {
		if (par < 0) {
			std::cerr << "Init only takes arguments >= 0\n\n";
			return false;
		}
		for (int i = 0; i < par; ++i) a.rt_stack.push_back(0);
		a.pc++;
		return true;
	}

	bool am1::ret(am1& a, int par) {
		if (par < 0 || a.rt_stack.size() < (size_t) (par + 2)) {
			std::cerr << "Not enough arguments on runtime stack\n\n";
			return false;
		}
		if (!a.ra_address_is_valid(a.rt_stack[a.rt_stack.size() - 2]) ||
			a.rt_stack.back() > ((int) a.ref - 2) || a.rt_stack.back() < 0) {
			std::cerr << "Can't return. Invalid arguments on runtime stack\n\n";
			return false;
		}
		a.pc = a.rt_stack[a.rt_stack.size() - 2];
		a.ref = a.rt_stack.back();
		for (int i = 0; i < (par + 2); ++i) a.rt_stack.pop_back();
		return true;
	}
}
