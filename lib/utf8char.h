#ifndef HAUNTED_LIB_UTF8CHAR_H_
#define HAUNTED_LIB_UTF8CHAR_H_

#include <cstdint>
#include <string>

namespace haunted {
	class utf8char {
		public:
			uint32_t cp;
			utf8char(uint32_t cp): cp(cp) {}
			utf8char(): cp(0) {}
			utf8char(const char *, size_t = 1, size_t = 0);
			utf8char(const std::string &str): utf8char(str.c_str(), str.size(), static_cast<size_t>(0)) {}

			operator uint32_t() const;
			operator std::string() const;
			size_t width() const;

			friend std::ostream & operator<<(std::ostream &os, const utf8char &input);
			friend std::string operator+(const std::string &, const utf8char &);
			friend std::string operator+(const char *, const utf8char &);
	};
}

#endif
