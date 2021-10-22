#ifndef HAUNTED_CORE_KEYS_H_
#define HAUNTED_CORE_KEYS_H_

#include <bitset>
#include <string>
#include <unordered_map>

#include <iostream>
#include "haunted/core/Defs.h"

namespace Haunted {
	/**
	 * Represents a key code.
	 */
	enum class KeyType: int {
		     Mouse =  -1,
		      Bell =   7,        BS,          Tab,  Enter,
		     Space =  32,      Bang,       DQuote,   Hash,      Dollar,  Percent, Ampersand, SQuote,
		    OParen =  40,    CParen,         Star,   Plus,       Comma,    Minus,    Period,  Slash,
		     Colon =  58, Semicolon,    OpenAngle, Equals,  CloseAngle, Question,        At,
		OpenSquare =  91, Backslash,  CloseSquare,  Caret,  Underscore,     Tick,
		 OpenCurly = 123,      Pipe,   CloseCurly,  Tilde,         Del,
		        _0 =  48, _1, _2, _3, _4, _5, _6, _7, _8, _9,
		         A =  65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		         a =  97, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
		CarriageReturn = 13, Escape = 27, LessThan = 60,  GreaterThan = 62, Backspace = 127,

		// Here be dragons.
		Home = 128, End, Insert,
		F1 = 131, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		UpArrow, DownArrow, RightArrow, LeftArrow, PageUp, PageDown
	};

	/**
	 * Represents a keyboard modifier in the CSI u representation (but 0-based).
	 * For mouse reports, what's called alt here actually represents the meta key,
	 * as alt-clicks don't seem to be reported.
	 */
	enum class KeyMod: int {None = 0, Shift = 1, Alt = 2, Ctrl = 4};

	using ModSet = std::bitset<3>;

	/**
	 * Represents a keypress, including any modifiers.
	 */
	struct Key {
		private:
			static std::unordered_map<KeyType, std::string> keymap;

		public:
			KeyType type;
			ModSet mods;

			Key(KeyType type, ModSet mods): type(type), mods(mods) {}
			Key(KeyType type, KeyMod mod):  Key(type, getModSet(mod)) {}
			Key(KeyType type):              Key(type, KeyMod::None) {}

			Key(int t, ModSet mods): Key(KeyType(t), mods) {}
			Key(int t, KeyMod mod):  Key(KeyType(t), mod) {}
			Key(int t):              Key(t,    KeyMod::None) {}
			Key():                   Key('\0', KeyMod::None) {}

			static ModSet getModSet(KeyMod);

			/** Returns true if this key's type is equal to a given key type and this key's only modifier is control. */
			bool isCtrl(KeyType) const;

			/** Returns true if this key's type is equal to a given key type and this key's only modifier is alt. */
			bool isAlt(KeyType) const;

			/** Returns true if this key's type is equal to a given key type and this key's only modifier is shift. */
			bool isShift(KeyType) const;

			/** Returns true if this key's type is equal to a given key type and this key's only modifiers are shift and
			 *  control. */
			bool isCtrlShift(KeyType) const;

			/** Returns true if the key is one of the arrow keys. */
			bool isArrow() const;

			/** Returns a copy of the key with the shift modifier set. */
			Key shift() const;

			/** Returns a copy of the key with the alt modifier set. */
			Key alt() const;

			/** Returns a copy of the key with the control modifier set. */
			Key ctrl() const;

			bool hasShift() const;
			bool hasAlt() const;
			bool hasCtrl() const;

			/** Returns false if the key is null/invalid. */
			operator bool() const;

			/** Converts the key to a char. If the key is greater than 127, this returns zero. */
			operator char() const;

			/** Returns the key's value. */
			operator int() const;

			/** Returns a string representation of the key and its modifiers. Useful mostly for debugging purposes. */
			operator std::string() const;

			/** Returns whether the key is equal to a character, but case-insensitively. */
			bool operator%(int) const;

			/** Returns whether the key is identical to another key. The key type and modifiers must match. */
			bool operator==(const Key &) const;
			bool operator!=(const Key &right) const { return !(*this == right); }

			/** Returns whether the key matches a character. Case sensitive. */
			bool operator==(char) const;
			bool operator!=(char right) const { return !(*this == right); }

			bool operator==(KeyMod) const;
			bool operator!=(KeyMod right) const { return !(*this == right); }

			/** Returns whether the key is of a given type and has no modifiers. */
			bool operator==(KeyType) const;
			bool operator!=(KeyType right) const { return !(*this == right); }

			/** Returns whether the key has a given type, regardless of modifiers. */
			bool operator%(KeyType) const;

			bool operator&(KeyMod) const;

			friend std::ostream & operator<<(std::ostream &, const Key &);

			/** Returns a string representing a set of modifiers. */
			static std::string modString(const ModSet &);
	};

	bool operator==(KeyMod, const ModSet &);
	bool operator==(const ModSet &, KeyMod);
}

#endif
