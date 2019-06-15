#ifndef HAUNTED_CORE_UTIL_H_
#define HAUNTED_CORE_UTIL_H_

#include <string>
#include <utility>

namespace haunted {
	class util {
		public:
			/** Returns true if the character is in the range [A-Za-z]. */
			static bool is_alpha(char);

			/** Returns true if the character is in the range [0-9]. */
			static bool is_numeric(char);

			/** Returns true if the character is in the range [A-Za-z0-9]. */
			static bool is_alphanum(char);

			/** Returns true if the character is in the range [0x40, 0x7e]. */
			static bool is_finalchar(char);

			/** Returns true if the character is in a closed interval. */
			static bool in_range(char, char min, char max);
	};
}

#endif
