#include <iostream>

#include "core/key.h"
#include "core/defs.h"

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
		return mods == get_modset(mod);
	}

	bool key::operator&(kmod mod) const {

		DBG("Testing " << mods.to_string() << " vs. " << int(mod) << " on " << int(type));

		switch (int(mod)) {
			case int(kmod::shift): return mods.test(0);
			case int(kmod::alt):   return mods.test(1);
			case int(kmod::ctrl):  return mods.test(2);
			default: return !mods.any();
		}
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
		{ktype::home,        "⭶"},
		{ktype::end,         "⭸"},
		{ktype::insert,      "Ins"},
		{ktype::page_up,     "⭻"},
		{ktype::page_down,   "⭽"},
		{ktype::f1,          "F1"},
		{ktype::f2,          "F2"},
		{ktype::f3,          "F3"},
		{ktype::f4,          "F4"},
		{ktype::f5,          "F5"},
		{ktype::f6,          "F6"},
		{ktype::f7,          "F7"},
		{ktype::f8,          "F8"},
		{ktype::f9,          "F9"},
		{ktype::f10,         "F10"},
		{ktype::f11,         "F11"},
		{ktype::f12,         "F12"},
	};

	modset key::get_modset(kmod mod) {
		switch (int(mod)) {
			case int(kmod::shift): return modset().set(0);
			case int(kmod::alt):   return modset().set(1);
			case int(kmod::ctrl):  return modset().set(2);
			default: return modset();
		}
	}
}
