#include <map>
#include <vector>
#include <utility>
#include <iostream>
#include <boost/variant.hpp>
#include <functional>

namespace am0_interpreter {
	typedef struct am0_state {
		int unsigned pc;
		std::vector<int> d_stack;
		std::map<int,int> mem;
	} am0_state_t;

	class am0 {
		public:
			enum class func {
				ADD,SUB,MUL,DIV,MOD,LT,EQ,NE,GT,LE,GE, LOAD,LIT,STORE,JMP,JMC,READ,WRITE
			};

			void add_function(func, int = 0);
			bool set_state(const am0_state_t&);
			bool run(bool = false);
			void reset(void);
			friend std::ostream& operator<<(std::ostream&,const am0&);
		private:
			typedef boost::variant<
				std::function<bool(am0&)>,
				std::pair<std::function<bool(am0&,int)>,int>
			> am0_func;

			std::vector<am0_func> prog;

			class am0_func_visitor : public boost::static_visitor<bool> {
				public:
					am0_func_visitor(am0& a) : am0_machine(a) {}
					bool operator()(std::function<bool(am0&)>&);
					bool operator()(std::pair<std::function<bool(am0&,int)>,int>&);
				private:
					am0& am0_machine;
			};

			static bool load(am0&,int);
			static bool store(am0&,int);
			static bool read(am0&,int);
			static bool write(am0&,int);
		protected:
			unsigned int pc = 1;
			std::vector<int> d_stack;
			std::map<int,int> mem;

			bool enough_arguments_on_stack(int) const;
			bool address_is_valid(int,bool = false) const;
			bool jmp_address_is_valid(int,bool = false) const;

			bool perform_bin_op(const std::function<void(int,int&)>&);
			static bool add(am0&);
			static bool sub(am0&);
			static bool mul(am0&);
			static bool div(am0&);
			static bool mod(am0&);
			static bool lt(am0&);
			static bool eq(am0&);
			static bool ne(am0&);
			static bool gt(am0&);
			static bool le(am0&);
			static bool ge(am0&);
			static bool lit(am0&,int);
			static bool jmp(am0&,int);
			static bool jmc(am0&,int);
	};

	bool parse(am0&, std::istream& = std::cin, bool = false);
	bool parse(am0_state_t&, std::istream& = std::cin);
}
