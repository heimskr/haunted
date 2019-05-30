#ifndef HAUNTED_CORE_DEFS_H_
#define HAUNTED_CORE_DEFS_H_

#include <cstddef>

namespace haunted {
	enum side {left, right, top, bottom};

	struct point {
		size_t x, y;
		point(size_t x_, size_t y_): x(x_), y(y_) {}
	};

	struct position {
		int left, top, width, height;
		position(int l, int t, int w, int h): left(l), top(t), width(w), height(h) {}
		position(): left(-1), top(-1), width(-1), height(-1) {}
		operator bool() const;
	};
}

#endif
