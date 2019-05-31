#include <iostream>
#include <stdexcept>
#include <string>

#include <unistd.h>

#include "core/terminal.h"
#include "core/key.h"
#include "core/util.h"

namespace haunted {
	terminal::terminal(std::istream &in_stream_): in_stream(in_stream_) {
		original = attrs = getattr();
	}

	terminal::terminal(): terminal(std::cin) {}

	terminal::~terminal() {
		reset();
	}

	// Private static methods

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

	// Private instance methods

	void terminal::apply() {
		setattr(attrs);
	}

	void terminal::reset() {
		setattr(original);
		attrs = original;
	}

	// Public instance methods

	void terminal::cbreak() {
		attrs.c_lflag &= ~(ECHO | ICANON | ISIG);
		attrs.c_iflag &= ~IXON;
		apply();
	}

	// Public operators

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
		// If we receive an escape followed by a [ and another escape,
		// we return Alt+[ after receiving the second escape, but this
		// discards the second escape. To make up for this, we use a
		// static bool to indicate that this weird sequence has occurred.
		static bool partial_escape = false;

		char c;
		if (raw) {
			in_stream >> c;
			k = c;
			return *this;
		}

		k = 0;

		if (!(in_stream >> c))
			return *this;

		// It's important to reset the partial_escape flag. Resetting it right
		// after reading it prevents me from having to insert a reset before
		// every return statement.
		bool escape = partial_escape || c == key_type::escape;
		partial_escape = false;

		if (escape) {
			// If we read an escape byte, that means something interesting is about to happen.

			if (!(in_stream >> c))
				return *this;

			if (c == key_type::escape) {
				// We can't tell the difference between an actual press of the escape key and
				// the beginning of a CSI. Perhaps it would be possible with the use of some
				// timing trickery, but I don't consider that necessary right now. Instead,
				// the user will have to press the escape key twice.
				k = {c, none};
				return *this;
			} else if (c == key_type::open_square) {
				if (!(in_stream >> c))
					return *this;

				switch (c) {
					// To input an actual Alt+[, the user has to press the [ key again.
					// Otherwise, we wouldn't be able to tell the difference between an
					// actual Alt+[ and the beginning of a CSI.
					case '[':
						k = {c, alt};
						return *this;

					// If there's another escape immediately after "^[", we'll assume the
					// user typed an actual Alt+[ and then input another escape sequence.
					case key_type::escape:
						partial_escape = false;
						k = {'[', alt};
						return *this;

					// If the first character after the [ is A, B, C or D, it's an arrow key.
					case 'A': k = key_type::up;    return *this;
					case 'B': k = key_type::down;  return *this;
					case 'C': k = key_type::right; return *this;
					case 'D': k = key_type::left;  return *this;
				}

				// At this point, we haven't yet determined what the input is.
				// If I understand correctly, a CSI sequence ends with an upper-
				// or lower-case letter. Let's read until we encounter one.
				static std::string buffer;
				buffer = c;

				while (!util::is_alpha(c)) {
					if (!(in_stream >> c))
						return *this;

					buffer += c;
				}

				std::cout << "buffer = \"" << buffer << "\"[" << buffer.size() << "]. " << util::is_csiu(buffer) << std::endl;
			}

			k = {c, alt};
		} else {
			k = c;
		}

		return *this;
	}
}
