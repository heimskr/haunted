#ifndef HAUNTED_CORE_DEFS_H_
#define HAUNTED_CORE_DEFS_H_

// #define NODEBUG

#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>

#include <cstddef>

#include "formicine/ansi.h"

#define DBGT(x) DBG(ansi::style::bold << get_id(true) << "  " << ansi::action::reset << x)
#define DBGTFN() { std::string pfn = __PRETTY_FUNCTION__; size_t _sp = pfn.find_first_of(' '), _co = pfn.find_last_of( \
	':'), _lp = pfn.find_first_of('('); DBGT("\e[1m" << pfn.substr(0, _sp) << "\e[0;2m" << pfn.substr(_sp, _co + 1 -   \
	_sp) << "\e[0;1m" << pfn.substr(_co + 1, _lp - _co - 1) << "\e[0m" << pfn.substr(_lp)); }
// #define DBGFN() { std::string pfn = __PRETTY_FUNCTION__; DBG(ansi::style::bold << this << ": " << ansi::action::reset  \
// 	<< pfn.substr(0, pfn.find_first_of(' '))); }
#define DBGFN() DBG(ansi::style::bold << this << ": " << ansi::action::reset << __PRETTY_FUNCTION__)
#ifdef NODEBUG
#define DBGX(x)
#define DBG(x)
#else
#define DBGX(x) "\e[2m[" << std::right << std::setw(25) << std::setfill(' ') << std::string(__FILE__).substr(0, 25) << ":" \
	<< std::setw(3) << __LINE__ << "]\e[0m " << x << std::endl
#define DBG(x) haunted::dbgstream << DBGX(x) << ansi::action::reset
#endif

namespace haunted {
	extern std::ofstream dbgout;
	extern ansi::ansistream dbgstream;

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

		friend std::ostream & operator<<(std::ostream &, const position &);
	};

	template <typename T>
	class null_terminal: public std::runtime_error {
		private:
			T *source;

			std::runtime_error create_runtime_error(T *source) {
				char msg[1024];
				if (source == nullptr)
					sprintf(msg, "Terminal is null");
				else
					sprintf(msg, "Terminal is null for %p", source);
				return std::runtime_error(msg);
			}

		public:
			null_terminal(T *source): std::runtime_error(null_terminal::create_runtime_error(source)) {}
	};
}

namespace haunted::ui {
	class control;
	class child;
}

#endif
