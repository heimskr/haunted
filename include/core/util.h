#ifndef HAUNTED_CORE_UTIL_H_
#define HAUNTED_CORE_UTIL_H_

#include <string>
#include <utility>

namespace haunted {
	class util {
		public:
			/** Parses a CSI u sequence. Returns {-1, -k} if the input is invalid. */
			static std::pair<int, int> parse_csiu(const std::string &);
			/** Determines whether a string looks like a CSI u sequence (numbers, semicolon,
			 *  numbers, letter). */
			static bool is_csiu(const std::string &);
			/** Returns true if the character is in the range [A-Za-z]. */
			static bool is_alpha(char);
			/** Returns true if the character is in the range [0-9]. */
			static bool is_numeric(char);
			/** Returns true if the character is in the range [A-Za-z0-9]. */
			static bool is_alphanum(char);
			/** Returns true if the character is in the range [0x40, 0x7e]. */
			static bool is_finalchar(char);
	};
}

#endif
