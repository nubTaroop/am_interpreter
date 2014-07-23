#include <map>
#include <vector>
#include <utility>
#include <iostream>
#include <boost/variant.hpp>
#include <functional>

namespace am0_interpreter {
	class am0 {
		public:
			virtual bool run(bool = false); //starts the machine
			virtual void reset(void); //sets the machine state to default
			virtual bool parse_prog(std::istream& = std::cin, bool = false); //parse code into the machine
			virtual bool parse_state(std::istream& = std::cin); //parse a initial state to the machine
			virtual ~am0() {}
			friend std::ostream& operator<<(std::ostream&,const am0&); //print out the state of the machine
		private:
			typedef boost::variant<
				std::function<bool(am0&)>,
				std::pair<std::function<bool(am0&,int)>,int>
			> am0_func;

			class am0_func_visitor : public boost::static_visitor<bool> {
				public:
					am0_func_visitor(am0& a) : am0_machine(a) {}
					bool operator()(std::function<bool(am0&)>&);
					bool operator()(std::pair<std::function<bool(am0&,int)>,int>&);
				private:
					am0& am0_machine;
			};

			std::vector<am0_func> prog; //program code container
			std::map<int,int> mem; //memory: relation between memory addresses and memory values


			bool address_is_valid(int,bool = false) const; //check if a given memory address is valid

			static bool load(am0&,int), store(am0&,int);
			static bool read(am0&,int), write(am0&,int);
		protected:
			unsigned int pc = 1; //program counter
			std::vector<int> d_stack; //data stack

			virtual bool jmp_address_is_valid(int,bool = false) const; //check if a jump address is valid
			bool enough_arguments_on_stack(int) const; //check if enough arguments are on data stack

			bool perform_bin_op(const std::function<void(int,int&)>&); //perform a binary operation on data stack
			static bool add(am0&), sub(am0&), mul(am0&), div(am0&), mod(am0&);
			static bool lt(am0&), eq(am0&), ne(am0&), gt(am0&), le(am0&), ge(am0&);
			static bool lit(am0&,int), jmp(am0&,int), jmc(am0&,int);
	};

}
