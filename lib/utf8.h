#ifndef HAUNTED_LIB_UTF8_H_
#define HAUNTED_LIB_UTF8_H_

#include <cstdlib>

#include "superstring.h"

namespace haunted {
	class utf8 {
		public:
			/** Determines the expected codepoint width for a given start byte.
			 *  @return The expected width in bytes if the byte is a valid starting byte; 0 otherwise. */
			static size_t width(unsigned char);
	};
}

#endif
