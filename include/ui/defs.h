#ifndef HAUNTED_UI_DEFS_H_
#define HAUNTED_UI_DEFS_H_

#include <cstddef>

namespace haunted {
	enum side {left, right};

	struct point {
		size_t x, y;
		point(size_t x_, size_t y_): x(x_), y(y_) {}
	};

	struct rect {
		int x, y, w, h;
		rect(int x_, int y_, int w_, int h_): x(x_), y(y_), w(w_), h(h_) {}
	};
}

#endif
