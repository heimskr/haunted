#include "core/key.h"

namespace haunted {
	bool key::is_ctrl(ktype other) const {
		return type == other && *this == kmod::ctrl;
	}

	bool key::is_alt(ktype other) const {
		return type == other && *this == kmod::alt;
	}

	key::operator bool() const {
		return int(type) != '\0';
	}

	key::operator char() const {
		char c = static_cast<char>(type);
		return 127 < c? 0 : c;
	}

	key::operator int() const {
		return int(type);
	}

	key::operator std::string() const {
		std::string out;
		if (*this & kmod::ctrl) out.append("⌃");
		if (*this & kmod::alt)  out.append("⎇ ");
		auto found = keymap.find(type);
		if (found != keymap.end())
			out += found->second;
		else
			out += static_cast<char>(type);
		return out;
	}

	bool key::operator%(int right) const {
		char left = *this;
		if (left == right) return true;
		if (int(ktype::A) <= right && right <= int(ktype::Z)) return left == right + 32;
		if (int(ktype::a) <= right && right <= int(ktype::z)) return left == right - 32;
		return false;
	}

	bool key::operator==(const key &right) const {
		return type == right.type && mods == right.mods;
	}

	bool key::operator==(char right) const {
		return char(type) == right;
	}

	bool key::operator==(kmod mod) const {
		return mods.to_ulong() == static_cast<unsigned long>(mod);
	}

	bool key::operator&(kmod mod) const {
		return mods.test(int(mod));
	}

	std::ostream & operator<<(std::ostream &os, const key &k) {
		os << std::string(k);
		return os;
	}

	std::unordered_map<ktype, std::string> key::keymap = {
		{ktype::carriage_return, "↩"},
		{ktype::up_arrow,    "↑"},
		{ktype::down_arrow,  "↓"},
		{ktype::right_arrow, "→"},
		{ktype::left_arrow,  "←"},
		{ktype::enter,       "⌅"},
		{ktype::tab,         "⇥"},
		{ktype::escape,      "⎋"},
		{ktype::backspace,   "⌫"},
		{ktype::space,       "␣"},
	};
}
