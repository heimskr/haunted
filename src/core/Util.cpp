#include <iostream>

#include "haunted/core/Key.h"
#include "haunted/core/Util.h"

namespace Haunted {
	bool Util::isAlpha(char c) {
		return inRange(c, 'A', 'Z') || inRange(c, 'a', 'z');
	}

	bool Util::isNumeric(char c) {
		return inRange(c, '0', '9');
	}

	bool Util::isAlphanum(char c) {
		return isAlpha(c) || isNumeric(c);
	}

	bool Util::isFinalchar(char c) {
		return inRange(c, 0x40, 0x7e);
	}

	bool Util::inRange(char c, char min, char max) {
		return min <= c && c <= max;
	}
}
