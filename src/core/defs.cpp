#include "core/defs.h"

namespace haunted {
	point::operator bool() const {
		return 0 <= x && 0 <= y;
	}

	position::operator bool() const {
		return 0 <= width && 0 <= height;
	}
}
