#include "am0_interpreter.hpp"

namespace am1_interpreter {
	typedef struct am1_state {
		unsigned int pc;
		std::vector<int> d_stack;
		std::vector<int> rt_stack;
	} am1_state_t;

	class am1 : private am0_interpreter::am0 {
		public:
			bool set_state(const am0_interpreter::am0_state_t&) = delete;
			bool set_state(const am1_state_t&);
			bool run(bool = false);
			void reset(void);
			static bool parse(am0_interpreter::am0&, std::istream& = std::cin, bool = false) = delete;
			static bool parse(am0_interpreter::am0_state_t&, std::istream& = std::cin) = delete;
			static bool parse(am1&, std::istream& = std::cin, bool = false);
			static bool parse(am1_state_t&, std::istream& = std::cin);
			friend std::ostream& operator<<(std::ostream&,const am0&);
		private:
			typedef boost::variant<
				std::function<bool(am1&)>,
				std::pair<std::function<bool(am1&,int)>,int>,
				std::tuple<std::function<bool(am1&,bool,int)>, bool, int>
			> am1_func;

			std::vector<am1_func> prog;
			std::vector<int> rt_stack;

			class am1_func_visitor : public boost::static_visitor<bool> {
				public:
					am1_func_visitor(am1& a) : am1_machine(a) {}
					bool operator()(std::function<bool(am1&)>&);
					bool operator()(std::pair<std::function<bool(am1&,int)>,int>&);
					bool operator()(std::tuple<std::function<bool(am1&,int)>,bool,int>&);
				private:
					am1& am1_machine;
			};

			//TODO: new AM1_functions
	};
}
