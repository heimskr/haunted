#include "utf8.h"

namespace haunted {
	utf8char::utf8char(const char *str, size_t len, size_t offset): cp(0) {
		for (size_t i = 0; i < len && i < 4; ++i)
			cp |= static_cast<unsigned char>(str[offset + i]) << (8 * i);
	}

	utf8char::operator uint32_t() const {
		return cp;
	}

	utf8char::operator std::string() const {
		// TODO: change?
		const char chars[5] = {
			static_cast<char>(cp & 0xff),
			static_cast<char>((cp >> 8) & 0xff),
			static_cast<char>((cp >> 16) & 0xff),
			static_cast<char>((cp >> 24) & 0xff),
			0
		};

		return chars;
	}

	size_t utf8char::width() const {
		return utf8::width(cp);
	}
}
