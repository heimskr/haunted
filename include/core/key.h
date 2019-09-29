#ifndef HAUNTED_CORE_KEYS_H_
#define HAUNTED_CORE_KEYS_H_

#include <bitset>
#include <string>
#include <unordered_map>

#include <iostream>
#include "core/hdefs.h"

namespace haunted {
	/**
	 * Represents a key code.
	 */
	enum class ktype: int {
		       bell =   7,        bs,          tab,  enter,
		      space =  32,      bang,       dquote,   hash,      dollar,  percent, ampersand, squote,
		     oparen =  40,    cparen,         star,   plus,       comma,    minus,    period,  slash,
		      colon =  58, semicolon,   open_angle, equals, close_angle, question,        at,
		open_square =  91, backslash, close_square,  caret,  underscore,     tick,
		 open_curly = 123,      pipe,        ccurl,  tilde,         del,
		         _0 =  48, _1, _2, _3, _4, _5, _6, _7, _8, _9,
		          A =  65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		          a =  97, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
		carriage_return = 13, escape = 27, less_than = 60,  greater_than = 62, backspace = 127,

		// Here be dragons.
		home = 128, end, insert,
		f1 = 131, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
		up_arrow, down_arrow, right_arrow, left_arrow, page_up, page_down
	};

	/**
	 * Represents a keyboard modifier in the CSI u representation (but 0-based).
	 */
	enum class kmod {none = 0, shift = 1, alt = 2, ctrl = 4};

	using modset = std::bitset<3>;

	/**
	 * Represents a keypress, including any modifiers.
	 */
	struct key {
		private:
			static std::unordered_map<ktype, std::string> keymap;

		public:
			ktype type;
			modset mods;

			key(ktype type, modset mods): type(type), mods(mods) {}
			key(ktype type, kmod mod):    key(type, get_modset(mod)) {}
			key(ktype type):              key(type, kmod::none) {}

			key(int t, modset mods): key(ktype(t), mods) {}
			key(int t, kmod mod):    key(ktype(t), mod) {}
			key(int t):              key(t,        kmod::none) {}
			key():                   key('\0',     kmod::none) {}

			static modset get_modset(kmod);

			/** Returns true if this key's type is equal to a given key type and this key's only modifier is control. */
			bool is_ctrl(ktype) const;

			/** Returns true if this key's type is equal to a given key type and this key's only modifier is alt. */
			bool is_alt(ktype) const;

			/** Returns true if this key's type is equal to a given key type and this key's only modifier is shift. */
			bool is_shift(ktype) const;

			/** Returns true if the key is one of the arrow keys. */
			bool is_arrow() const;

			/** Returns a copy of the key with the shift modifier set. */
			key shift() const;

			/** Returns a copy of the key with the alt modifier set. */
			key alt() const;

			/** Returns a copy of the key with the control modifier set. */
			key ctrl() const;

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
			bool operator==(const key &) const;
			bool operator!=(const key &right) const { return !(*this == right); }

			/** Returns whether the key matches a character. Case sensitive. */
			bool operator==(char) const;
			bool operator!=(char right) const { return !(*this == right); }

			bool operator==(kmod) const;
			bool operator!=(kmod right) const { return !(*this == right); }

			/** Returns whether the key is of a given type and has no modifiers. */
			bool operator==(ktype) const;
			bool operator!=(ktype right) const { return !(*this == right); }

			/** Returns whether the key has a given type, regardless of modifiers. */
			bool operator%(ktype) const;

			bool operator&(kmod) const;

			friend std::ostream & operator<<(std::ostream &, const key &);
	};
}

#endif
