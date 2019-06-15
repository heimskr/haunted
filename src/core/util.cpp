#include <iostream>

#include "core/key.h"
#include "core/util.h"

namespace haunted {
	bool util::is_alpha(char c) {
		return in_range(c, 'A', 'Z') || in_range(c, 'a', 'z');
	}

	bool util::is_numeric(char c) {
		return in_range(c, '0', '9');
	}

	bool util::is_alphanum(char c) {
		return is_alpha(c) || is_numeric(c);
	}

	bool util::is_finalchar(char c) {
		return in_range(c, 0x40, 0x7e);
	}

	bool util::in_range(char c, char min, char max) {
		return min <= c && c <= max;
	}
}
