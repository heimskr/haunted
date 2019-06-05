#include "core/key.h"

namespace haunted {
	key_modifier key::make_modifier(bool shift, bool alt, bool ctrl) {
		return key_modifier((shift * key_modifier::shift)
		                  | (alt   * key_modifier::alt)
		                  | (ctrl  * key_modifier::ctrl));
	}

	bool key::is_ctrl(key_type other) const {
		return type == other && mod == ctrl;
	}

	bool key::is_alt(key_type other) const {
		return type == other && mod == alt;
	}

	key::operator bool() const {
		return type != '\0';
	}

	key::operator char() const {
		char c = static_cast<char>(type);
		return 127 < c? 0 : c;
	}

	key::operator int() const {
		return type;
	}

	key::operator std::string() const {
		std::string out;
		if (mod & ctrl) out.append("⌃");
		if (mod & alt)  out.append("⎇ ");
		auto found = keymap.find(type);
		if (found != keymap.end())
			out += found->second;
		else
			out += static_cast<char>(type);
		return out;
	}

	bool key::operator%(char right) const {
		char left = *this;
		if (left == right) return true;
		if (key_type::A <= right && right <= key_type::Z) return left == right + 32;
		if (key_type::a <= right && right <= key_type::z) return left == right - 32;
		return false;
	}

	bool key::operator==(const key &right) const {
		return type == right.type && mod == right.mod;
	}

	bool key::operator==(char right) const {
		return type == right;
	}

	std::ostream & operator<<(std::ostream &os, const key &k) {
		os << std::string(k);
		return os;
	}

	std::unordered_map<key_type, std::string> key::keymap = {
		{key_type::carriage_return, "↩"},
		{key_type::up_arrow,    "↑"},
		{key_type::down_arrow,  "↓"},
		{key_type::right_arrow, "→"},
		{key_type::left_arrow,  "←"},
		{key_type::enter,       "⌅"},
		{key_type::tab,         "⇥"},
		{key_type::escape,      "⎋"},
		{key_type::backspace,   "⌫"},
		{key_type::space,       "␣"},
	};
}
