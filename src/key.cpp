#include "haunted/key.h"

namespace haunted {
	/**
	 * Converts the key to a character.
	 * If the key is greater than 127, this returns zero.
	 */
	key::operator char() const {
		char c = static_cast<char>(type);
		return 127 < c? 0 : c;
	}

	/**
	 * Determines whether the key is equal to a character, but case-insensitively.
	 * This is always false for characters or keys above 127.
	 */
	bool key::operator<=>(char right) const {
		char left = *this;
		if (127 < left || 127 < right) return false;
		if (left == right) return true;
		if (key_type::A <= right && right <= key_type::Z) return left == right + 32;
		if (key_type::a <= right && right <= key_type::z) return left == right - 32;
		return false;
	}
}
