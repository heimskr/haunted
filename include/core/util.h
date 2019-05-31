#ifndef HAUNTED_CORE_UTIL_H_
#define HAUNTED_CORE_UTIL_H_

#include <string>
#include <utility>

namespace haunted {
	class util {
		public:
			static std::pair<int, int> parse_csiu(const std::string &);
			static bool is_csiu(const std::string &);
			static bool is_alpha(char);
			static bool is_numeric(char);
			static bool is_alphanum(char);
	};
}

#endif
