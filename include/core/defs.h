#ifndef HAUNTED_CORE_DEFS_H_
#define HAUNTED_CORE_DEFS_H_

#include <cstddef>

#include "../../lib/formicine/ansi.h"

#define DBGX(x) "\e[2m[" << __FILE__ << ":" << __LINE__ << "]\e[0m " << x << std::endl
#define DBG(x) ansi::ansistream::err() << DBGX(x) << ansi::reset

namespace haunted {
	enum class side {left, right, top, bottom};

	struct point {
		int x, y;

		point(int x, int y): x(x), y(y) {}
		point(): point(-1, -1) {}

		operator bool() const;
	};

	/** Represents a position. This is zero-based, while ANSI is one-based, so be careful. */
	struct position {
		int left, top, width, height;
		
		position(int l, int t, int w, int h): left(l), top(t), width(w), height(h) {}
		position(): position(-1, -1, -1, -1) {}
		
		/** Moves the cursor to a given offset from the top-left corner of the position. */
		void jump(int offset_left = 0, int offset_top = 0);

		/** Returns the rightmost column of the position. */
		int right() const;

		/** Returns the bottommost column of the position. */
		int bottom() const;

		bool operator==(const position &other) const;

		/** Returns whether this is a valid (i.e., not default-constructed) position. */
		explicit operator bool() const;

		/** Returns a string representation of the position. */
		operator std::string() const;
	};
}

#endif
