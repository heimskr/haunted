#ifndef HAUNTED_CORE_UTIL_H_
#define HAUNTED_CORE_UTIL_H_

#include <memory>
#include <string>
#include <utility>

#include <cxxabi.h>

namespace Haunted {
	class Util {
		public:
			/** Returns true if the character is in the range [A-Za-z]. */
			static bool isAlpha(char);

			/** Returns true if the character is in the range [0-9]. */
			static bool isNumeric(char);

			/** Returns true if the character is in the range [A-Za-z0-9]. */
			static bool isAlphanum(char);

			/** Returns true if the character is in the range [0x40, 0x7e]. */
			static bool isFinalchar(char);

			/** Returns true if the character is in a closed interval. */
			static bool inRange(char, char min, char max);

			/** Demangles a type name. */
			static std::string demangle(const std::string &mangled) {
				const char * const name = mangled.c_str();
				int status = 0;

				std::unique_ptr<char, void(*)(void *)> result {
					abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free
				};

				return status == 0? result.get() : name;
			}

			/** Returns the demangled name of an object. */
			template <typename T>
			static std::string demangleObject(const T &object) {
				return demangle(std::string(typeid(object).name()));
			}

			template <typename T>
			static std::string hex(T n) {
				std::stringstream ss;
				ss << std::hex << n;
				return ss.str();
			}
	};
}

#define EXTENDS(sub, base) typename sub, typename std::enable_if<std::is_base_of<base, sub>::value>::type * = nullptr
#define BEGIN_END(source) \
	decltype(source)::              iterator   begin() { return (source).begin();   } \
	decltype(source)::      reverse_iterator  rbegin() { return (source).rbegin();  } \
	decltype(source)::        const_iterator  cbegin() { return (source).cbegin();  } \
	decltype(source)::const_reverse_iterator crbegin() { return (source).crbegin(); } \
	decltype(source)::              iterator     end() { return (source).end();     } \
	decltype(source)::      reverse_iterator    rend() { return (source).rend();    } \
	decltype(source)::        const_iterator    cend() { return (source).cend();    } \
	decltype(source)::const_reverse_iterator   crend() { return (source).crend();   }

#endif
