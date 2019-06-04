#ifndef HAUNTED_CORE_DEFS_H_
#define HAUNTED_CORE_DEFS_H_

#include <cstddef>

namespace haunted {
	enum side {left, right, top, bottom};

	struct point {
		int x, y;

		point(int x_, int y_): x(x_), y(y_) {}
		point(): point(-1, -1) {}

		operator bool() const;
	};

	struct position {
		int left, top, width, height;
		
		position(int l, int t, int w, int h):
			left(l), top(t), width(w), height(h) {}
		
		position():
			left(-1), top(-1), width(-1), height(-1) {}
		
		operator bool() const;
	};
}

#endif
