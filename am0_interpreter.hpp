#include <map>
#include <vector>
#include <utility>
#include <iostream>
#include <boost/variant.hpp>
#include <functional>

namespace am0_interpreter {
	class am0 {
		public:
			virtual bool run(bool = false);
			virtual void reset(void);
			virtual bool parse_prog(std::istream& = std::cin, bool = false);
			virtual bool parse_state(std::istream& = std::cin);
			virtual ~am0() {}
			friend std::ostream& operator<<(std::ostream&,const am0&);
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

			std::vector<am0_func> prog;
			std::map<int,int> mem;


			bool address_is_valid(int,bool = false) const;

			static bool load(am0&,int), store(am0&,int);
			static bool read(am0&,int), write(am0&,int);
		protected:
			unsigned int pc = 1;
			std::vector<int> d_stack;

			virtual bool jmp_address_is_valid(int,bool = false) const;
			bool enough_arguments_on_stack(int) const;

			bool perform_bin_op(const std::function<void(int,int&)>&);
			static bool add(am0&), sub(am0&), mul(am0&), div(am0&), mod(am0&);
			static bool lt(am0&), eq(am0&), ne(am0&), gt(am0&), le(am0&), ge(am0&);
			static bool lit(am0&,int), jmp(am0&,int), jmc(am0&,int);
	};

}
