#include "lib/uutil.h"

namespace Haunted {
	bool uutil::is_regional_indicator(uint32_t ch) {
		return 0x1f1e6 <= ch && ch <= 0x1f1ff;
	}

	bool uutil::is_regional_indicator(int ch) {
		return is_regional_indicator(static_cast<uint32_t>(ch));
	}
}
