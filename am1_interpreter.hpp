#include "am0_interpreter.hpp"

namespace am1_interpreter {
	class am1 : private am0_interpreter::am0 {
		public:
			bool run(bool = false) final override;
			void reset(void) final override;
			bool parse_prog(std::istream& = std::cin, bool = false) final override;
			bool parse_state(std::istream& = std::cin) final override;
			friend std::ostream& operator<<(std::ostream&,const am0&);
		private:
			typedef boost::variant<
				std::function<bool(am1&)>,
				std::pair<std::function<bool(am1&,int)>,int>,
				std::tuple<std::function<bool(am1&,bool,int)>, bool, int>
			> am1_func;

			std::vector<am1_func> prog;
			std::vector<int> rt_stack;
			unsigned int ref;

			enum state {local, global};

			class am1_func_visitor : public boost::static_visitor<bool> {
				public:
					am1_func_visitor(am1& a) : am1_machine(a) {}
					bool operator()(std::function<bool(am1&)>&);
					bool operator()(std::pair<std::function<bool(am1&,int)>,int>&);
					bool operator()(std::tuple<std::function<bool(am1&,state,int)>,state,int>&);
				private:
					am1& am1_machine;
			};

			static bool load(am1&,state,int), store(am1&,state,int);
		   	static bool read(am1&,state,int), write(am1&,state,int);
			static bool loadi(am1&,int), storei(am1&,int);
			static bool readi(am1&,int), writei(am1&,int);
			static bool loada(am1&,state,int);
			static bool push(am1&);
			static bool call(am1&,int), init(am1&,int), ret(am1&,int);
	};
}
