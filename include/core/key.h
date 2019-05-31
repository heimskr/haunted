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
		up = 128, down, right, left
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

		public:
			key_type type;
			key_modifier mod;

			key(key_type type, key_modifier mod): type(type), mod(mod) {}
			key(key_type t, bool ctrl, bool alt):
				key(t, key_modifier((ctrl * key_modifier::ctrl) | (alt * key_modifier::alt))) {}
			key(key_type t): key(t, none) {}

			key(int t, key_modifier mod): key(key_type(t), mod) {}
			key(int t, int mod): key(t, key_modifier(mod)) {}
			key(int t): key(key_type(t), none) {}
			key(): key(key_type('\0'), none) {}

			operator bool() const;
			operator char() const;
			operator int() const;
			operator std::string() const;
			bool operator%(char) const;
			bool operator==(const key &) const;
			bool operator==(char) const;

			friend std::ostream & operator<<(std::ostream &, const key &);
	};
}

#endif
