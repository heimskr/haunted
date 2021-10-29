#ifndef HAUNTED_CORE_DEFS_H_
#define HAUNTED_CORE_DEFS_H_

// #define NODEBUG

#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>

#include <cstddef>

#include "lib/formicine/ansi.h"

#define DBGT(x) DBG(ansi::style::bold << getID(true) << "  " << ansi::action::reset << x)
#define DBGTFNC() { std::string pfn = __PRETTY_FUNCTION__; size_t _sp = pfn.find_first_of(' '), _co = pfn.find_last_of( \
	':'), _lp = pfn.find_first_of('('); DBGT("\e[1m" << pfn.substr(0, _sp) << "\e[0;2m" << pfn.substr(_sp, _co + 1 -   \
	_sp) << "\e[0;1m" << pfn.substr(_co + 1, _lp - _co - 1) << "\e[0m" << pfn.substr(_lp)); }
#define DBGTFN() DBGT(ansi::dim(__PRETTY_FUNCTION__))
#define DBGFN() DBG(ansi::style::bold << this << ": " << ansi::action::reset << __PRETTY_FUNCTION__)

namespace Haunted {
	extern std::ofstream dbgout;
	extern ansi::ansistream dbgstream;

	enum class Side {Left, Right, Top, Bottom};

	struct Point {
		ssize_t x, y;

		Point(ssize_t x, ssize_t y): x(x), y(y) {}
		Point(): Point(-1, -1) {}

		operator bool() const;
	};

	/** Represents a position. This is zero-based, while ANSI is one-based, so be careful. */
	struct Position {
		ssize_t left, top, width, height;
		
		Position(ssize_t l, ssize_t t, ssize_t w, ssize_t h): left(l), top(t), width(w), height(h) {}
		Position(): Position(-1, -1, -1, -1) {}
		
		/** Moves the cursor to a given offset from the top-left corner of the position. */
		void jump(ssize_t offset_left = 0, ssize_t offset_top = 0);

		/** Returns the rightmost column of the position. */
		ssize_t right() const;

		/** Returns the bottommost column of the position. */
		ssize_t bottom() const;

		bool operator==(const Position &) const;
		bool operator!=(const Position &) const;

		/** Returns whether this is a valid (i.e., not default-constructed) position. */
		explicit operator bool() const;

		/** Returns a string representation of the position. */
		operator std::string() const;

	};

	template <typename T>
	class NullTerminal: public std::runtime_error {
		private:
			T *source;

			std::runtime_error createRuntimeError(T *source) {
				return std::runtime_error("Terminal is null " + (source? "for " + hex(source) : ""));
			}

		public:
			NullTerminal(T *source): std::runtime_error(NullTerminal::createRuntimeError(source)) {}
	};

	std::ostream & operator<<(std::ostream &, const Haunted::Position &);
}

namespace Haunted::UI {
	class Control;
	class Child;
}

#endif
