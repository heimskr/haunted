#include "lib/ansi.h"
#include "core/defs.h"

namespace haunted {
	point::operator bool() const {
		return 0 <= x && 0 <= y;
	}

	void position::jump(int inner_offset_left, int inner_offset_top) {
		ansi::jump(top + 1 + inner_offset_top, left + 1 + inner_offset_left);
	}

	position::operator bool() const {
		return 0 <= width && 0 <= height;
	}
}
