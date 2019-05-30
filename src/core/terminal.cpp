#include <iostream>
#include <stdexcept>
#include <string>

#include <unistd.h>

#include "core/terminal.h"
#include "core/key.h"

namespace haunted {
	terminal::terminal(std::istream &in_stream_): in_stream(in_stream_) {
		original = attrs = getattr();
	}

	terminal::terminal(): terminal(std::cin) {}

	terminal::~terminal() {
		reset();
	}

	termios terminal::getattr() {
		termios out;
		int result;
		if ((result = tcgetattr(STDIN_FILENO, &out)) < 0)
			throw std::runtime_error("tcgetattr returned " + std::to_string(result));

		return out;
	}

	void terminal::setattr(const termios &new_attrs) {
		int result;
		if ((result = tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_attrs)) < 0)
			throw std::runtime_error("tcsetattr returned " + std::to_string(result));
	}

	void terminal::apply() {
		setattr(attrs);
	}

	void terminal::reset() {
		setattr(original);
		attrs = original;
	}

	void terminal::cbreak() {
		attrs.c_lflag &= ~(ECHO | ICANON | ISIG);
		attrs.c_iflag &= ~IXON;
		apply();
	}

	terminal::operator bool() const {
		return bool(in_stream);
	}

	terminal & terminal::operator>>(char &ch) {
		char c = 0;
		if (in_stream >> c)
			ch = c;
		return *this;
	}

	terminal & terminal::operator>>(key &k) {
		char c;
		if (raw) {
			in_stream >> c;
			k = c;
			return *this;
		}

		k = 0;
		if (!(in_stream >> c)) {
			return *this;
		} else if (c == key_type::alt) {
			if (!(in_stream >> c)) {
				return *this;
			} else if (c == key_type::open_square) {
				if (!(in_stream >> c)) {
					return *this;
				} else if (key_type::A <= c && c <= key_type::D) {
					k = key_type::up + c - key_type::A;
					return *this;
				}
			}
			k = {c, false, true};
		} else {
			k = c;
		}

		return *this;
	}
}
