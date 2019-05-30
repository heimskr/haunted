#include "core/key.h"

namespace haunted {
	/**
	 * Returns whether the key is null/invalid.
	 */
	key::operator bool() const {
		return type != '\0';
	}

	/**
	 * Converts the key to a character.
	 * If the key is greater than 127, this returns zero.
	 */
	key::operator char() const {
		char c = static_cast<char>(type);
		return 127 < c? 0 : c;
	}

	/**
	 * Returns the key's value.
	 */
	key::operator int() const {
		return type;
	}

	/**
	 * Returns whether the key is equal to a character, but case-insensitively.
	 */
	bool key::operator<=>(char right) const {
		char left = *this;
		if (left == right) return true;
		if (key_type::A <= right && right <= key_type::Z) return left == right + 32;
		if (key_type::a <= right && right <= key_type::z) return left == right - 32;
		return false;
	}

	/**
	 * Returns whether the key is identical to another key. The key type and modifiers must match.
	 */
	bool key::operator==(const key &right) const {
		return type == right.type && ctrl == right.ctrl && alt == right.alt;
	}

	/**
	 * Returns whether the key matches a character. Case sensitive.
	 */
	bool key::operator==(char right) const {
		return type == right;
	}
}
