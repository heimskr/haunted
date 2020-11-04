#include "lib/UUtil.h"

namespace Haunted {
	bool UUtil::isRegionalIndicator(uint32_t ch) {
		return 0x1f1e6 <= ch && ch <= 0x1f1ff;
	}

	bool UUtil::isRegionalIndicator(int ch) {
		return isRegionalIndicator(static_cast<uint32_t>(ch));
	}
}
