#include <iostream>
#include <stdexcept>
#include <string>

#include <sys/ioctl.h>
#include <unistd.h>

#include "core/terminal.h"
#include "core/key.h"
#include "core/util.h"
#include "lib/ansi.h"
#include "ui/child.h"

namespace haunted {
	std::vector<terminal *> terminal::winch_targets {};

	terminal::terminal(std::istream &in_stream, ansi::ansistream out_stream):
	in_stream(in_stream), out_stream(out_stream) {
		original = attrs = getattr();
		winsize size;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
		rows = size.ws_row;
		cols = size.ws_col;
	}

	terminal::~terminal() {
		reset();
		if (input_thread.joinable())
			input_thread.join();
	}


// Private static methods


	/**
	 * Returns the terminal attributes from tcgetaddr.
	 */
	termios terminal::getattr() {
		termios out;
		int result;
		if ((result = tcgetattr(STDIN_FILENO, &out)) < 0)
			throw std::runtime_error("tcgetattr returned " +
				std::to_string(result));

		return out;
	}

	/**
	 * Sets the terminal attributes with tcsetaddr.
	 */
	void terminal::setattr(const termios &new_attrs) {
		int result;
		if ((result = tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_attrs)) < 0)
			throw std::runtime_error("tcsetattr returned " +
				std::to_string(result));
	}

	/**
	 * Notifies terminal objects of a window resize.
	 */
	void terminal::winch_handler(int) {
		winsize new_size;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &new_size);
		for (terminal *ptr: winch_targets)
			ptr->winch(new_size.ws_row, new_size.ws_col);
	}


// Private instance methods


	/**
	 * Applies the terminal attribute settings.
	 */
	void terminal::apply() {
		setattr(attrs);
	}

	/**
	 * Resets the terminal attributes to what they were before any changes were
	 * applied.
	 */
	void terminal::reset() {
		setattr(original);
		attrs = original;
	}

	/**
	 * Repeatedly reads from the terminal and sends the key press
	 * to the focused control.
	 */
	void terminal::work_input() {
		key k;
		cbreak();
		while (*this >> k) {
			if (k == key(key_type::c, true, false))
				break;

			send_key(k);
		}
	}

	ui::keyhandler * terminal::send_key(key k) {
		// If the root is null, there are no controls
		// and nothing to send key presses to.
		if (root == nullptr)
			return nullptr;

		ui::control *ctrl = get_focused();

		if (ctrl == nullptr)
			throw std::runtime_error("Focused control is null");

		if (ctrl->on_key(k))
			return ctrl;

		ui::container *ptr = ctrl->get_parent();
		
		// Keep trying on_key, going up to the root as long as
		// we keep getting false. If we're at the root and on_key
		// still returns false, give up.
		while (!ptr->on_key(k) && dynamic_cast<ui::control *>(ptr) != root) {
			if (ui::child *cptr = dynamic_cast<ui::child *>(ptr)) {
				ptr = cptr->get_parent();
			} else {
				return nullptr;
			}
		}

		return ptr;
	}

	void terminal::winch(int new_rows, int new_cols) {
		bool changed = rows != new_rows || cols != new_cols;
		rows = new_rows;
		cols = new_cols;
		if (changed)
			redraw();
	}


// Public instance methods


	void terminal::cbreak() {
		attrs.c_lflag &= ~(ECHO | ICANON | ISIG);
		attrs.c_iflag &= ~IXON;
		apply();
	}

	void terminal::watch_size() {
		if (winch_targets.empty())
			signal(SIGWINCH, &terminal::winch_handler);
		winch_targets.push_back(this);
	}

	void terminal::redraw() {
		if (root) {
			ansi::clear();
			ansi::jump();
			root->resize({0, 0, cols, rows});
			root->draw();
		}
	}

	void terminal::set_root(ui::control *new_root) {
		// If new_root is already the root, we need to return here.
		// Otherwise, we'd set the root to a dangling pointer and
		// then try to call methods on it from terminal::redraw().
		if (root == new_root)
			return;

		delete root;
		root = new_root;
		redraw();
	}

	void terminal::draw() {
		if (root)
			root->draw();
	}

	void terminal::start_input() {
		input_thread = std::thread(&terminal::work_input, this);
	}

	void terminal::focus(ui::control *to_focus) {
		focused = to_focus;
	}

	ui::control * terminal::get_focused() {
		if (focused)
			return focused;
		focused = root;
		return root;
	}

	bool terminal::add_child(ui::control *child) {
		set_root(child);
		return true;
	}

	terminal * terminal::get_term() {
		return this;
	}

	int terminal::get_rows() const {
		return rows;
	}

	int terminal::get_cols() const {
		return cols;
	}


// Public operators


	terminal::operator bool() const {
		return bool(in_stream);
	}

	terminal & terminal::operator>>(int &ch) {
		if (int c = in_stream.get())
			ch = c;
		return *this;
	}

	terminal & terminal::operator>>(char &ch) {
		char c = 0;
		if (in_stream.get(c))
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
			*this >> c;
			k = c;
			return *this;
		}

		k = 0;

		if (!(*this >> c))
			return *this;

		// It's important to reset the partial_escape flag. Resetting it right
		// after reading it prevents me from having to insert a reset before
		// every return statement.
		bool escape = partial_escape || c == key_type::escape;
		partial_escape = false;

		if (escape) {
			// If we read an escape byte, that means something interesting is
			// about to happen.

			if (!(*this >> c))
				return *this;

			if (c == key_type::escape) {
				// We can't tell the difference between an actual press of the
				// escape key and the beginning of a CSI. Perhaps it would be
				// possible with the use of some timing trickery, but I don't
				// consider that necessary right now. Instead, the user will
				// have to press the escape key twice.
				k = {c, none};
				return *this;
			} else if (c == key_type::open_square) {
				if (!(*this >> c))
					return *this;

				switch (c) {
					// To input an actual Alt+[, the user has to press the
					// [ key again. Otherwise, we wouldn't be able to tell the
					// difference between an actual Alt+[ and the beginning of
					// a CSI.
					case '[':
						k = {c, alt};
						return *this;

					// If there's another escape immediately after "^[", we'll
					// assume the user typed an actual Alt+[ and then input
					// another escape sequence.
					case key_type::escape:
						partial_escape = false;
						k = {'[', alt};
						return *this;

					// If the first character after the [ is A, B, C or D,
					// it's an arrow key.
					case 'A': k = key_type::up_arrow;    return *this;
					case 'B': k = key_type::down_arrow;  return *this;
					case 'C': k = key_type::right_arrow; return *this;
					case 'D': k = key_type::left_arrow;  return *this;
				}

				// At this point, we haven't yet determined what the input is.
				// If I understand correctly, a CSI sequence ends with an upper-
				// or lower-case letter. Let's read until we encounter one.
				static std::string buffer;
				buffer = c;

				while (!util::is_alpha(c)) {
					if (!(*this >> c))
						return *this;

					buffer += c;
				}

				std::pair<int, int> csiu_pair = util::parse_csiu(buffer);
				int codepoint = csiu_pair.first, mods = csiu_pair.second;
				if (codepoint >= 0) {
					k = {codepoint, (mods - 1) & 7};
					return *this;
				}
			}

			k = {c, alt};
		} else if (c == 9) {
			k = {key_type::tab};
		} else if (c == 10) {
			k = {key_type::enter};
		} else if (c == 13) {
			k = {key_type::carriage_return};
		} else if (0 < c && c < 27) {
			// 1..26 corresponds to ^a..^z.
			k = {key_type::a + c - 1, ctrl};
		} else {
			k = c;
		}

		return *this;
	}
}
