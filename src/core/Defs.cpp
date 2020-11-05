#include "haunted/core/Defs.h"

namespace Haunted {
	std::ofstream dbgout(".log", std::ofstream::app);
	ansi::ansistream dbgstream(dbgout, dbgout);

	Point::operator bool() const {
		return 0 <= x && 0 <= y;
	}

	void Position::jump(int inner_offset_left, int inner_offset_top) {
		ansi::out.jump(left + inner_offset_left, top + inner_offset_top);
	}

	int Position::right() const {
		return left + width - 1;
	}

	int Position::bottom() const {
		return top + height - 1;
	}

	bool Position::operator==(const Position &other) const {
		return left == other.left && top == other.top && width == other.width && height == other.height;
	}

	bool Position::operator!=(const Position &other) const {
		return left != other.left || top != other.top || width != other.width || height != other.height;
	}

	Position::operator bool() const {
		return 0 <= width && 0 <= height;
	}

	Position::operator std::string() const {
		return "[(" + std::to_string(left) + ", " + std::to_string(top) + "), " + std::to_string(width) + " x " +
			std::to_string(height) + "]";
	}

	std::ostream & operator<<(std::ostream &os, const Haunted::Position &pos) {
		return os << std::string(pos);
	}
}
