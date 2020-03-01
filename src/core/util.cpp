#include <iostream>

#include "haunted/core/key.h"
#include "haunted/core/util.h"

namespace Haunted {
	bool Util::is_alpha(char c) {
		return in_range(c, 'A', 'Z') || in_range(c, 'a', 'z');
	}

	bool Util::is_numeric(char c) {
		return in_range(c, '0', '9');
	}

	bool Util::is_alphanum(char c) {
		return is_alpha(c) || is_numeric(c);
	}

	bool Util::is_finalchar(char c) {
		return in_range(c, 0x40, 0x7e);
	}

	bool Util::in_range(char c, char min, char max) {
		return min <= c && c <= max;
	}
}
