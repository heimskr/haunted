#ifndef HAUNTED_CORE_UTIL_H_
#define HAUNTED_CORE_UTIL_H_

#include <memory>
#include <string>
#include <utility>

#include <cxxabi.h>

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

			/** Returns the demangled name of an object. */
			template <typename T>
			static std::string demangle(const T &object) {
				const char * const name = typeid(object).name();
				int status = 0;

				std::unique_ptr<char, void(*)(void *)> result = {
					abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free
				};

				return status == 0? result.get() : name;
			}
	};
}

#endif
