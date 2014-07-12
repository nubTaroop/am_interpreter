#include <iostream>
#include <sstream>
#include <string>
#include "am0_interpreter.hpp"

namespace am0_interpreter {
	bool am0::enough_arguments_on_stack(int amount) const {
		if (d_stack.size() < (size_t) amount) {
			std::cerr << "Not enough arguments on data stack" << std::endl;
			return false;
		}
		return true;
	}

	bool am0::address_is_valid(int address, bool check_load) const {
		if (address < 0 || (check_load && !mem.count(address))) {
			std::cerr << "Invalid memory address" << std::endl;
			return false;
		}
		return true;
	}

	bool am0::jmp_address_is_valid(int jmp_address, bool check_loop) const {
		if (jmp_address < 0 || (size_t) jmp_address > prog.size()) {
			std::cerr << "Invalid jump address" << std::endl;
			return false;
		}
		if (check_loop && (unsigned int) jmp_address == pc) {
			std::cerr << "Loop jump" << std::endl;
			return false;
		}
		return true;
	}

	bool am0::set_state( const am0_state_t& state) {
		if (!jmp_address_is_valid(state.pc)) return false;
		this->reset();
		pc = state.pc;
		mem = state.mem;
		d_stack = state.d_stack;
		return true;
	}

	void am0::reset() {
		pc = 1;
		d_stack.clear();
		mem.clear();
	}

	bool am0::am0_func_visitor::operator()(std::function<bool(am0&)>& f) {
		return f(this->am0_machine);
	}

	bool am0::am0_func_visitor::operator()(std::pair<std::function<bool(am0&,int)>, int>& fc) {
		return fc.first(this->am0_machine,fc.second);
	}

	bool am0::run(bool logging) {
		am0_func_visitor afv {*this};
		while (pc && (pc <= prog.size())) {
			if (logging) std::cout << *this << std::endl;
			if (boost::apply_visitor(afv,prog[pc - 1])) continue;
			else return false;
		}
		if (pc) { std::cerr << "Program counter ran out of line" << std::endl; return false; }
		return true;
	}

	std::ostream& operator<<(std::ostream& os,const am0& o) {
		std::string ret = "{" + std::to_string(o.pc) + ",";
		for (auto rit = o.d_stack.rbegin(); rit != o.d_stack.rend(); ++rit) ret += std::to_string(*rit) + ":";
		if (o.d_stack.size()) ret.pop_back();
		else ret += "-";
		ret += ",[";
		for (auto x : o.mem ) ret += std::to_string(x.first) + "/" + std::to_string(x.second) + ",";
		if (o.mem.size()) ret.pop_back();
		ret += "]}";
		return os << ret;
	}

	bool am0::perform_bin_op(const std::function<void(int,int&)>& f) {
		f(d_stack[d_stack.size() - 1], d_stack[d_stack.size() - 2]);
		d_stack.pop_back();
		++pc;
		return true;
	}

	bool am0::add(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		return a.perform_bin_op([] (int first, int& second) { second += first; });
	}

	bool am0::sub(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		return a.perform_bin_op([] (int first, int& second) { second -= first; });
	}

	bool am0::mul(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		return a.perform_bin_op([] (int first, int& second) { second *= first; });
	}

	bool am0::div(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		if (a.d_stack.back()) { return a.perform_bin_op([] (int first, int& second) { second /= first; }); }
		else { std::cerr << "Null division" << std::endl; return false; }
	}

	bool am0::mod(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		if (a.d_stack.back()) { return a.perform_bin_op([] (int first, int& second) { second %= first; }); }
		else { std::cerr << "Null division" << std::endl; return false;}
	}

	bool am0::lt(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		return a.perform_bin_op([] (int first, int& second) { second = second < first; });
	}

	bool am0::eq(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		return a.perform_bin_op([] (int first, int& second) { second = second == first; });
	}

	bool am0::ne(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		return a.perform_bin_op([] (int first, int& second) { second = second != first; });
	}

	bool am0::gt(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		return a.perform_bin_op([] (int first, int& second) { second = second > first; });
	}

	bool am0::le(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		return a.perform_bin_op([] (int first, int& second) { second = second <= first; });
	}

	bool am0::ge(am0& a) {
		if (!a.enough_arguments_on_stack(2)) return false;
		return a.perform_bin_op([] (int first, int& second) { second = second >= first; });
	}

	bool am0::load(am0& a, int par) {
		if (!a.address_is_valid(par,true)) return false;
		a.d_stack.push_back(a.mem[par]);
		++a.pc;
		return true;
	}

	bool am0::lit(am0& a, int par) {
		a.d_stack.push_back(par);
		++a.pc;
		return true;
	}

	bool am0::store(am0& a, int par) {
		if (!a.address_is_valid(par) || !a.enough_arguments_on_stack(1)) return false;
		a.mem[par] = a.d_stack.back();
		a.d_stack.pop_back();
		++a.pc;
		return true;
	}

	bool am0::jmp(am0& a, int par) {
		if (!a.jmp_address_is_valid(par, true)) return false;
		a.pc = par;
		return true;
	}

	bool am0::jmc(am0& a, int par) {
		if (!a.jmp_address_is_valid(par) || !a.enough_arguments_on_stack(1)) return false;
		if (a.d_stack.back() == 0) a.pc = par;
		else if (a.d_stack.back() == 1) ++a.pc;
		else { std::cerr << "Jump conditions have to be 1 or 0" << std::endl; return false; }
		a.d_stack.pop_back();
		return true;
	}

	bool am0::read(am0& a, int par) {
		if (!a.address_is_valid(par)) return false;
		int i;
		std::cout << " In: "; std::cin >> i;
		if (std::cin.fail()) { std::cin.clear(); std::cerr << "Wrong input" << std::endl; return false; }
		a.mem[par] = i;
		++a.pc;
		return true;
	}

	bool am0::write(am0& a, int par) {
		if (!a.address_is_valid(par, true)) return false;
		std::cout << "Out: " << a.mem[par] << std::endl;
		++a.pc;
		return true;
	}

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

	bool am0::parse(am0& am, std::istream& is, bool file) {
		std::cout << "AM0 code:" << std::endl;
		int lnr = 0;
		std::string line;
		while (std::cout << std::to_string(++lnr) << ": " && std::getline(is,line)) {
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
			if (line == "") {
				--lnr;
				if (file) std::cout << "\x1b[0G";
				else std::cout << "\x1b[F";
				std::cout << "\x1b[K\n";
				continue;
			}
#endif
			if (file) std::cout << line << std::endl;
			std::stringstream ls {line};
			std::string keyword;
			int par;
			ls >> keyword;
			if (keyword == "ADD;") { am.prog.push_back(add); continue; }
			else if (keyword == "SUB;") { am.prog.push_back(sub); continue; }
			else if (keyword == "MUL;") { am.prog.push_back(mul); continue; }
			else if (keyword == "DIV;") { am.prog.push_back(div); continue; }
			else if (keyword == "MOD;") { am.prog.push_back(mod); continue; }
			else if (keyword == "LT;") { am.prog.push_back(lt); continue; }
			else if (keyword == "EQ;") { am.prog.push_back(eq); continue; }
			else if (keyword == "NE;") { am.prog.push_back(ne); continue; }
			else if (keyword == "GT;") { am.prog.push_back(gt); continue; }
			else if (keyword == "LE;") { am.prog.push_back(le); continue; }
			else if (keyword == "GE;") { am.prog.push_back(ge); continue; }
			else if (keyword == "LOAD") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				am.prog.push_back(std::make_pair(load,par)); continue; }
			}
			else if (keyword == "LIT") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				am.prog.push_back(std::make_pair(lit,par)); continue; }
			}
			else if (keyword == "STORE") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				am.prog.push_back(std::make_pair(store,par)); continue; }
			}
			else if (keyword == "JMP") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				am.prog.push_back(std::make_pair(jmp,par)); continue; }
			}
			else if (keyword == "JMC") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				am.prog.push_back(std::make_pair(jmc,par)); continue; }
			}
			else if (keyword == "READ") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				am.prog.push_back(std::make_pair(read,par)); continue; }
			}
			else if (keyword == "WRITE") { if (ls.get() == ' ' && ls >> par && ls.get() == ';') {
				am.prog.push_back(std::make_pair(write,par)); continue; }
			}
			return parse_error(is);
		}
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
		std::cout << "\x1b[0G\x1b[0K";
#endif
		std::cout << std::endl;
		is.clear();
		return true;
	}

	bool am0::parse(am0_state_t& state, std::istream& is) {
		state.d_stack.clear();
		state.mem.clear();
		std::cout << "AM0 state:" << std::endl;
		std::string line;
		std::getline(is,line);
		if (is.fail()) return parse_error(is);
		std::istringstream cs {line};
		if ( !((cs.get() == '{') && (cs >> state.pc) && (cs.get() == ',')) ) return parse_error(cs,line);
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
				if ( !((cs.get() == ',') && (cs >> key) && (cs.get() == '/') && (cs >> value)) ) return parse_error(cs,line);
				else {
					if (state.mem.count(key)) return parse_error(cs,line);
					else state.mem[key] = value;
				}
			}
		}
		cs.ignore();
		if (cs.get() != '}') return parse_error(cs,line);
		std::cout << std::endl;
		return true;
	}
}
