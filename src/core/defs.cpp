#include "haunted/core/defs.h"

namespace Haunted {
	std::ofstream dbgout(".log", std::ofstream::app);
	ansi::ansistream dbgstream(dbgout, dbgout);

	point::operator bool() const {
		return 0 <= x && 0 <= y;
	}

	void position::jump(int inner_offset_left, int inner_offset_top) {
		ansi::out.jump(left + inner_offset_left, top + inner_offset_top);
	}

	int position::right() const {
		return left + width - 1;
	}

	int position::bottom() const {
		return top + height - 1;
	}

	bool position::operator==(const position &other) const {
		return left == other.left && top == other.top && width == other.width && height == other.height;
	}

	bool position::operator!=(const position &other) const {
		return left != other.left || top != other.top || width != other.width || height != other.height;
	}

	position::operator bool() const {
		return 0 <= width && 0 <= height;
	}

	position::operator std::string() const {
		return "[(" + std::to_string(left) + ", " + std::to_string(top) + "), " + std::to_string(width) + " x " +
			std::to_string(height) + "]";
	}

	std::ostream & operator<<(std::ostream &os, const position &pos) {
		os << std::string(pos);
		return os;
	}
}
