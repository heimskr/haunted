#include "formicine/ansi.h"
#include "core/defs.h"

namespace haunted {
	point::operator bool() const {
		return 0 <= x && 0 <= y;
	}

	void position::jump(int inner_offset_left, int inner_offset_top) {
		ansi::jump(left + inner_offset_left, top + inner_offset_top);
	}

	int position::right() const {
		return left + width - 1;
	}

	int position::bottom() const {
		return top + height - 1;
	}

	position::operator bool() const {
		return 0 <= width && 0 <= height;
	}
}
