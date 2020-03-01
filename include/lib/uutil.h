#ifndef HAUNTED_LIB_UUTIL_H_
#define HAUNTED_LIB_UUTIL_H_

#include <cstdint>

namespace haunted {
	struct uutil {
		static bool is_regional_indicator(uint32_t);
		static bool is_regional_indicator(int);
	};
}

#endif
