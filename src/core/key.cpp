#include <iostream>

#include "haunted/core/Key.h"

namespace Haunted {
	bool Key::isCtrl(KeyType other) const {
		return type == other && *this == KeyMod::Ctrl;
	}

	bool Key::isAlt(KeyType other) const {
		return type == other && *this == KeyMod::Alt;
	}

	bool Key::isShift(KeyType other) const {
		return type == other && *this == KeyMod::Shift;
	}

	bool Key::isArrow() const {
		return type == KeyType::UpArrow    || type == KeyType::DownArrow ||
		       type == KeyType::RightArrow || type == KeyType::LeftArrow;
	}

	Key Key::shift() const {
		return {type, ModSet(mods).set(0, true)};
	}

	Key Key::alt() const {
		return {type, ModSet(mods).set(1, true)};
	}

	Key Key::ctrl() const {
		return {type, ModSet(mods).set(2, true)};
	}

	Key::operator bool() const {
		return int(type) != '\0';
	}

	Key::operator char() const {
		return static_cast<char>(type);
	}

	Key::operator int() const {
		return int(type);
	}

	Key::operator std::string() const {
		auto found = keymap.find(type);
		if (found != keymap.end())
			return modString(mods) + found->second;
		return modString(mods) + static_cast<char>(type);
	}

	bool Key::operator%(int right) const {
		char left = *this;
		if (left == right) return true;
		if (int(KeyType::A) <= right && right <= int(KeyType::Z))
			return left == right + 32;
		if (int(KeyType::a) <= right && right <= int(KeyType::z))
			return left == right - 32;
		return false;
	}

	bool Key::operator==(const Key &right) const {
		return type == right.type && mods == right.mods;
	}

	bool Key::operator==(char right) const {
		return char(type) == right;
	}

	bool Key::operator==(KeyMod mod) const {
		return mods == getModSet(mod);
	}

	bool Key::operator==(KeyType right) const {
		return type == right && mods.none();
	}

	bool Key::operator%(KeyType right) const {
		return type == right;
	}

	bool Key::operator&(KeyMod mod) const {
		switch (int(mod)) {
			case int(KeyMod::Shift): return mods.test(0);
			case int(KeyMod::Alt):   return mods.test(1);
			case int(KeyMod::Ctrl):  return mods.test(2);
			default: return !mods.any();
		}
	}

	std::unordered_map<KeyType, std::string> Key::keymap = {
		{KeyType::CarriageReturn, "↩"},
		{KeyType::UpArrow,     "↑"},
		{KeyType::DownArrow,   "↓"},
		{KeyType::RightArrow,  "→"},
		{KeyType::LeftArrow,   "←"},
		{KeyType::Enter,       "⌅"},
		{KeyType::Tab,         "⇥"},
		{KeyType::Escape,      "⎋"},
		{KeyType::Backspace,   "⌫"},
		{KeyType::Space,       "␣"},
		{KeyType::Home,        "⭶"},
		{KeyType::End,         "⭸"},
		{KeyType::Insert,      "Ins"},
		{KeyType::PageUp,      "⭻"},
		{KeyType::PageDown,    "⭽"},
		{KeyType::F1,          "F1"},
		{KeyType::F2,          "F2"},
		{KeyType::F3,          "F3"},
		{KeyType::F4,          "F4"},
		{KeyType::F5,          "F5"},
		{KeyType::F6,          "F6"},
		{KeyType::F7,          "F7"},
		{KeyType::F8,          "F8"},
		{KeyType::F9,          "F9"},
		{KeyType::F10,         "F10"},
		{KeyType::F11,         "F11"},
		{KeyType::F12,         "F12"},
	};

	ModSet Key::getModSet(KeyMod mod) {
		switch (int(mod)) {
			case int(KeyMod::Shift): return ModSet().set(0);
			case int(KeyMod::Alt):   return ModSet().set(1);
			case int(KeyMod::Ctrl):  return ModSet().set(2);
			default: return ModSet();
		}
	}

	std::string Key::modString(const ModSet &mods) {
		std::string out;
		if (mods.test(0)) out.append("⇧");
		if (mods.test(2)) out.append("⌃");
		if (mods.test(1)) out.append("⎇ "); // the extra space is because the character is wide but still one column.
		return out;
	}

	bool operator==(KeyMod mod, const ModSet &mods) {
		switch (mod) {
			case KeyMod::Shift: return mods == 1;
			case KeyMod::Alt:   return mods == 2;
			case KeyMod::Ctrl:  return mods == 4;
			default: return false;
		}
	}

	bool operator==(const ModSet &mods, KeyMod mod) {
		return mod == mods;
	}

	std::ostream & operator<<(std::ostream &os, const Haunted::Key &k) {
		return os << std::string(k);
	}
}
