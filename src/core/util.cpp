#include <iostream>

#include "core/util.h"
#include "core/key.h"

namespace haunted {
	/** Determines whether a string looks like a CSI u sequence (numbers, semicolon, numbers, letter). */
	bool util::is_csiu(const std::string &str) {
		const size_t len = str.size();

		// There are four components in a CSI u sequence, so the string has to be at least 4 bytes.
		// It also needs to start with a number.
		if (len < 4 || !is_numeric(str[0]))
			return false;

		size_t i = 0;
		for (; i < len && is_numeric(str[i]); ++i);
		if (str[i++] != ';') return false;
		for (; i < len && is_numeric(str[i]); ++i);
		return i == len - 1 && str[i] == 'u';
	}

	/** Parses a CSI u sequence. Returns {-1, -k} if the input is invalid. */
	std::pair<int, int> util::parse_csiu(const std::string &str) {
		const ssize_t len = str.size();

		if (len < 4 || !is_numeric(str[0]))
			return {-1, -1};
		
		if (str[len - 1] != 'u')
			return {-1, -2};
		
		ssize_t p, i = len - 2;
		int first = 0, second = 0;

		for (p = 1; 0 <= i && is_numeric(str[i]); --i) {
			second += p * (str[i] - '0');
			p *= 10;
		}

		if (str[i--] != ';') return {-1, -3};

		for (p = 1; 0 <= i && is_numeric(str[i]); --i) {
			first += p * (str[i] - '0');
			p *= 10;
		}

		if (i == -1)
			return {first, second};

		return {-1, -4};
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
