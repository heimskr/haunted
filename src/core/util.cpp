#include "core/util.h"
#include "core/key.h"

namespace haunted {
	/** Determines whether a string looks like a CSI u sequence (numbers, semicolon, numbers, letter). */
	bool util::is_csiu(const std::string &str) {
		const size_t len = str.size();

		// There are four components in a CSI u sequence, so the string has to be at least 4 bytes.
		if (len < 4)
			return false;

		// It also needs to start with number.
		if (!is_numeric(str[0]))
			return false;

		size_t i = 0;
		for (; i < len && is_numeric(str[i]); ++i);
		if (str[i++] != ';') return false;
		for (; i < len && is_numeric(str[i]); ++i);
		return i == len - 1 && str[i] == 'u';
	}

	bool util::is_alpha(char c) {
		return (key_type::A <= c && c <= key_type::Z) || (key_type::a <= c && c <= key_type::z);
	}

	bool util::is_numeric(char c) {
		return key_type::_0 <= c && c <= key_type::_9;
	}

	bool util::is_alphanum(char c) {
		return is_alpha(c) || is_numeric(c);
	}
}
