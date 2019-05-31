#ifndef HAUNTED_CORE_UTIL_H_
#define HAUNTED_CORE_UTIL_H_

#include <string>

namespace haunted {
	class util {
		public:
			static bool is_csiu(const std::string &);
			static bool is_alpha(char);
			static bool is_numeric(char);
			static bool is_alphanum(char);
	};
}

#endif
