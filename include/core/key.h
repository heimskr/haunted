#ifndef HAUNTED_KEYS_H_
#define HAUNTED_KEYS_H_

namespace haunted {
	enum key_type {
		       bell =   7, backspace,          tab,  enter,
		      space =  32,      bang,       dquote,   hash,      dollar,  percent, ampersand, squote,
		     oparen =  40,    cparen,         star,   plus,       comma,    minus,    period,  slash,
		      colon =  58, semicolon,   open_angle, equals, close_angle, question,        at,
		open_square =  91, backslash, close_square,  caret,  underscore,     tick,
		 open_curly = 123,      pipe,        ccurl,  tilde,         del,
		         _0 =  48, _1, _2, _3, _4, _5, _6, _7, _8, _9,
		          A =  65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		          a =  97, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,

		carriage_return = 13, escape = 27, less_than = 60,  greater_than = 62
	};

	struct key {
		key_type type;
		bool ctrl, alt;

		key(key_type type_, bool ctrl_, bool alt_): type(type_), ctrl(ctrl_), alt(alt_) {}
		key(key_type type_): key(type_, false, false) {}

		operator char() const;
		bool operator<=>(char) const;
	};
}

#endif
