#ifndef HAUNTED_CORE_KEYS_H_
#define HAUNTED_CORE_KEYS_H_

#include <string>
#include <unordered_map>

namespace haunted {
	/**
	 * Represents a key code.
	 */
	enum key_type: int {
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
		up_arrow = 128, down_arrow, right_arrow, left_arrow
	};

	/**
	 * Represents a keyboard modifier in the CSI u representation (but 0-based).
	 */
	enum key_modifier {none = 0, shift = 1, alt = 2, ctrl = 4};

	/**
	 * Represents a keypress, including any modifiers.
	 */
	struct key {
		private:
			static std::unordered_map<key_type, std::string> keymap;
			static key_modifier make_modifier(bool shift, bool alt, bool ctrl);

		public:
			key_type type;
			key_modifier mod;

			key(key_type type, key_modifier mod): type(type), mod(mod) {}
			key(key_type t, bool ctrl, bool alt):
				key(t, key_modifier(make_modifier(false, alt, ctrl))) {}
			key(key_type t): key(t, none) {}

			key(int t, key_modifier mod): key(key_type(t), mod) {}
			key(int t, int mod): key(t, key_modifier(mod)) {}
			key(int t): key(key_type(t), none) {}
			key(): key(key_type('\0'), none) {}

			/** Returns true if this key's type is equal to a given key type and this key's only
			 *  modifier is control. */
			bool is_ctrl(key_type) const;
			/** Returns true if this key's type is equal to a given key type and this key's only
			 *  modifier is alt. */
			bool is_alt(key_type) const;

			/** Returns false if the key is null/invalid. */
			operator bool() const;
			/** Converts the key to a char. If the key is greater than 127, this returns zero. */
			operator char() const;
			/** Returns the key's value. */
			operator int() const;
			/** Returns a string representation of the key and its modifiers. Useful mostly for
			 *  debugging purposes. */
			operator std::string() const;
			/** Returns whether the key is equal to a character, but case-insensitively. */
			bool operator%(char) const;
			/** Returns whether the key is identical to another key. The key type and modifiers
			 *  must match. */
			bool operator==(const key &) const;
			/** Returns whether the key matches a character. Case sensitive. */
			bool operator==(char) const;

			friend std::ostream & operator<<(std::ostream &, const key &);
	};
}

#endif
