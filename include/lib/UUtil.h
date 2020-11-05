#ifndef HAUNTED_LIB_UUTIL_H_
#define HAUNTED_LIB_UUTIL_H_

#include <cstdint>

namespace Haunted {
	struct UUtil {
		static bool isRegionalIndicator(uint32_t);
		static bool isRegionalIndicator(int);
	};
}

#endif
