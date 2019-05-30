#include "core/defs.h"

namespace haunted {
	position::operator bool() const {
		return 0 <= width && 0 <= height;
	}
}
