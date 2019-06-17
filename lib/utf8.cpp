#include "utf8.h"

namespace haunted {
	size_t utf8::width(unsigned char uch) {
		if (uch < 0x80)           return 1;
		if ((uch & 0xe0) == 0xc0) return 2;
		if ((uch & 0xf0) == 0xe0) return 3;
		if ((uch & 0xf8) == 0xf0) return 4;
		return 0;
	}
}
