#include <iostream>
#include <stdexcept>
#include <string>

#include <unistd.h>

#include "haunted/terminal.h"

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
		attrs.c_lflag &= ~(ECHO | ICANON);
		apply();
	}

	terminal::operator bool() const {
		return bool(in_stream);
	}

	terminal & terminal::operator>>(char &ch) {
		char c = 0;
		if (in_stream >> c) {
			if (c == 27) {

			}

			ch = c;
		}

		return *this;
	}
}
