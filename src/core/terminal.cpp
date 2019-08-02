#include <iostream>
#include <stdexcept>
#include <string>

#include <csignal>
#include <cstdlib>
#include <sys/ioctl.h>
#include <unistd.h>

#include "core/csi.h"
#include "core/key.h"
#include "core/terminal.h"
#include "core/util.h"
#include "formicine/ansi.h"
#include "ui/child.h"

namespace haunted {
	using uchar = unsigned char;

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

	void terminal::winch_handler(int) {
		winsize new_size;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &new_size);
		for (terminal *ptr: winch_targets)
			ptr->winch(new_size.ws_row, new_size.ws_col);
	}


// Private instance methods


	void terminal::apply() {
		setattr(attrs);
	}

	void terminal::reset() {
		setattr(original);
		attrs = original;
	}

	void terminal::work_input() {
		key k;
		// Sometimes, calling cbreak() once doesn't seem to properly set all the flags (e.g., arrow keys produce strings
		// like "^[[C"). Calling it twice appears to work, but it's not pretty.
		cbreak();
		cbreak();
		while (*this >> k) {
			if (k == key(ktype::c, kmod::ctrl))
				break;

			send_key(k);
		}
	}

	ui::keyhandler * terminal::send_key(key k) {
		// If the root is null, there are no controls and nothing to send key presses to.
		if (root == nullptr)
			return nullptr;

		ui::control *ctrl = get_focused();

		if (ctrl == nullptr)
			throw std::runtime_error("Focused control is null");

		if (ctrl->on_key(k))
			return ctrl;

		ui::container *ptr = ctrl->get_parent();
		
		// Keep trying on_key, going up to the root as long as we keep getting false. If we're at the root and on_key
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
			out_stream.clear();
			out_stream.jump();
			root->resize({0, 0, cols, rows});
			root->draw();
		}
	}

	void terminal::set_root(ui::control *new_root) {
		// If new_root is already the root, we need to return here. Otherwise, we'd set the root to a dangling pointer
		// and then try to call methods on it from terminal::redraw().
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

	void terminal::flush() {
		out_stream.flush();
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

	bool terminal::has_focus(const ui::control *ctrl) const {
		return focused == ctrl;
	}

	int terminal::get_rows() const {
		return rows;
	}

	int terminal::get_cols() const {
		return cols;
	}

	position terminal::get_position() const {
		return {0, 0, get_cols(), get_rows()};
	}

	void terminal::jump(int x, int y) {
		if (0 <= x && 0 <= y) {
			*this << "\e[" << (y + 1) << ";" << (x + 1) << "H";
		} else if (0 <= x) {
			*this << "\e[" << (x + 1) << "G";
		} else if (0 <= y) {
			*this << "\e[999999A";
			if (0 < y)
				*this << "\e[" + std::to_string(y) + "B";
		} else {
			throw std::runtime_error("Invalid jump: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
		}
	}

	void terminal::vscroll(int rows) {
		if (0 < rows) {
			*this << "\e[" + std::to_string(rows) + "T";
		} else if (rows < 0) {
			*this << "\e[" + std::to_string(-rows) + "U";
		}
	}

	void terminal::hmargins(size_t left, size_t right) {
		out_stream << "\e[" + std::to_string(left + 1) + ";" + std::to_string(right + 1) + "s";
	}

	void terminal::hmargins() {
		out_stream << "\e[s";
	}

	void terminal::vmargins(size_t top, size_t bottom) {
		out_stream << "\e[" + std::to_string(top + 1) + ";" + std::to_string(bottom + 1) + "r";
	}

	void terminal::vmargins() {
		out_stream << "\e[r";
	}

	void terminal::margins(size_t top, size_t bottom, size_t left, size_t right) {
		vmargins(top, bottom);
		hmargins(left, right);
	}

	void terminal::margins() {
		hmargins();
		vmargins();
	}

	void terminal::enable_hmargins() { // DECLRMM: Left Right Margin Mode
		out_stream << "\e[?69h";
	}

	void terminal::disable_hmargins() {
		out_stream << "\e[?69l";
	}

	void terminal::set_origin() {
		out_stream << "\e[?6h";
	}

	void terminal::reset_origin() {
		out_stream << "\e[?6l";
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
		// If we receive an escape followed by a [ and another escape, we return Alt+[ after receiving the second
		// escape, but this discards the second escape. To make up for this, we use a static bool to indicate that this
		// weird sequence has occurred.
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

		// It's important to reset the partial_escape flag. Resetting it right after reading it prevents me from having
		// to insert a reset before every return statement.
		bool escape = partial_escape || c == uchar(ktype::escape);
		partial_escape = false;

		if (escape) {
			// If we read an escape byte, that means something interesting is about to happen.

			if (!(*this >> c))
				return *this;

			if (c == uchar(ktype::escape)) {
				// We can't tell the difference between an actual press of the/ escape key and the beginning of a CSI.
				// Perhaps it would be possible with the use of some timing trickery, but I don't consider that
				// necessary right now (YAGNI!). Instead, the user will have to press the escape key twice.
				k = {c, kmod::none};
				return *this;
			} else if (c == uchar(ktype::open_square)) {
				if (!(*this >> c))
					return *this;

				switch (c) {
					// To input an actual Alt+[, the user has to press the [ key again. Otherwise, we wouldn't be able
					// to tell the difference between an actual Alt+[ and the beginning of a CSI.
					case '[':
						k = {c, kmod::alt};
						return *this;

					// If there's another escape immediately after "^[", we'll assume the user typed an actual Alt+[ and
					// then input another escape sequence.
					case int(ktype::escape):
						partial_escape = false;
						k = {'[', kmod::alt};
						return *this;

					// If the first character after the [ is A, B, C or D, it's an arrow key.
					case 'A': k = ktype::up_arrow;    return *this;
					case 'B': k = ktype::down_arrow;  return *this;
					case 'C': k = ktype::right_arrow; return *this;
					case 'D': k = ktype::left_arrow;  return *this;
				}

				// At this point, we haven't yet determined what the input is. A CSI sequence ends with a character in
				// the range [0x40, 0x7e]. Let's read until we encounter one.
				static std::string buffer;
				buffer = c;

				while (!util::is_finalchar(c)) {
					if (!(*this >> c))
						return *this;

					buffer += c;
				}

				const csi parsed = buffer;
				if (parsed.first >= 0) {
					k = key(parsed.get_key(), modset((parsed.second - 1) & 7));
					return *this;
				}
			}

			k = {c, kmod::alt};
		} else if (c == 9) {
			k = {ktype::tab};
		} else if (c == 10) {
			k = {ktype::enter};
		} else if (c == 13) {
			k = {ktype::carriage_return};
		} else if (0 < c && c < 27) {
			// 1..26 corresponds to ^a..^z.
			k = {uchar(ktype::a) + c - 1, kmod::ctrl};
		} else {
			k = c;
		}

		return *this;
	}


// Public static methods


	bool terminal::is_iterm() {
		return std::string(std::getenv("TERM_PROGRAM")) == "iTerm.app";
	}
}
