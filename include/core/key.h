#ifndef HAUNTED_CORE_KEYS_H_
#define HAUNTED_CORE_KEYS_H_

#include <string>
#include <unordered_map>

namespace haunted {
	enum key_type: int {
		       bell =   7, backspace,          tab,  enter,
		      space =  32,      bang,       dquote,   hash,      dollar,  percent, ampersand, squote,
		     oparen =  40,    cparen,         star,   plus,       comma,    minus,    period,  slash,
		      colon =  58, semicolon,   open_angle, equals, close_angle, question,        at,
		open_square =  91, backslash, close_square,  caret,  underscore,     tick,
		 open_curly = 123,      pipe,        ccurl,  tilde,         del,
		         _0 =  48, _1, _2, _3, _4, _5, _6, _7, _8, _9,
		          A =  65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		          a =  97, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
		carriage_return = 13, alt = 27, escape = 27, less_than = 60,  greater_than = 62,

		// Here be dragons.
		up = 128, down, right, left,
	};

	struct key {
		private:
			static std::unordered_map<key_type, std::string> keymap;

		public:
			key_type type;
			bool ctrl, alt;

			key(key_type t, bool ctrl, bool alt): type(t), ctrl(ctrl), alt(alt) {}
			key(int t, bool ctrl, bool alt): key(key_type(t), ctrl, alt) {}
			key(key_type t): key(t, false, false) {}
			key(int t): key(key_type(t), false, false) {}
			key(): key('\0') {}

			operator bool() const;
			operator char() const;
			operator int() const;
			operator std::string() const;
			bool operator<=>(char) const;
			bool operator==(const key &) const;
			bool operator==(char) const;

			friend std::ostream & operator<<(std::ostream &, const key &);
	};
}

#endif
