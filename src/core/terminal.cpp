#include <deque>
#include <iostream>
#include <stdexcept>
#include <string>

#include <csignal>
#include <sys/ioctl.h>
#include <unistd.h>

#include "haunted/core/csi.h"
#include "haunted/core/key.h"
#include "haunted/core/terminal.h"
#include "haunted/core/util.h"
#include "haunted/ui/child.h"

#include "lib/formicine/futil.h"

namespace haunted {
	using uchar = unsigned char;

	std::vector<terminal *> terminal::winch_targets {};

	terminal::terminal(std::istream &in_stream_, ansi::ansistream &out_stream_):
	in_stream(in_stream_), out_stream(out_stream_), colors(&out_stream_, &output_mutex) {
		original = attrs = getattr();
		winsize size;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
		rows = size.ws_row;
		cols = size.ws_col;
	}

	terminal::~terminal() {
		if (!suppress_output) {
			out_stream.reset_colors();
			out_stream.clear();
			reset();
			join();
			jump(0, 0);
		}

		delete root;
	}


// Private static methods


	void terminal::winch_handler(int) {
		winsize new_size;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &new_size);
		for (terminal *ptr: winch_targets)
			ptr->winch(new_size.ws_row, new_size.ws_col);
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


// Private instance methods


	void terminal::apply() {
		setattr(attrs);
	}

	void terminal::reset() {
		mouse(mouse_mode::none);
		setattr(original);
		attrs = original;
		DBG("reset()");
	}

	void terminal::work_input() {
		key k;
		// Sometimes, calling cbreak() once doesn't seem to properly set all the flags (e.g., arrow keys produce strings
		// like "^[[C"). Calling it twice appears to work, but it's not pretty.
		cbreak();
		cbreak();
		while (*this >> k) {
			if (k == key(ktype::c, kmod::ctrl) && on_interrupt())
				break;

			send_key(k);
		}
	}

	void terminal::winch(int new_rows, int new_cols) {
		bool changed = rows != new_rows || cols != new_cols;
		rows = new_rows;
		cols = new_cols;
		if (changed) {
			std::unique_lock lock(winch_mutex);
			redraw();
		}
	}


// Public instance methods


	void terminal::cbreak() {
		attrs.c_lflag &= ~(ECHO | ICANON | ISIG);
		attrs.c_iflag &= ~IXON;
		apply();
	}

	void terminal::watch_size() {
		if (winch_targets.empty())
			std::signal(SIGWINCH, &terminal::winch_handler);
		winch_targets.push_back(this);
	}

	void terminal::redraw() {
		if (root) {
			colors.reset();
			out_stream.clear().jump();
			root->resize({0, 0, cols, rows});
		}
	}

	void terminal::set_root(ui::control *new_root, bool delete_old) {
		if (root != new_root) {
			if (delete_old)
				delete root;
			root = new_root;
			redraw();
		}
	}

	void terminal::draw() {
		if (root)
			root->draw();
	}

	void terminal::reset_colors() {
		colors.reset();
	}

	ui::keyhandler * terminal::send_key(const key &k) {
		// If the root is null, there are no controls and nothing to send key presses to.
		if (root == nullptr)
			return nullptr;

		ui::control *ctrl = get_focused();

		if (!ctrl)
			throw std::runtime_error("Focused control is null");

		if (ctrl->on_key(k)) {
			if (key_postlistener)
				key_postlistener(k);
			return ctrl;
		}

		ui::container *ptr = ctrl->get_parent();
		
		// Keep trying on_key, going up to the root as long as we keep getting false. If we're at the root and on_key
		// still returns false, let the terminal itself handle the keypress as a last resort.
		while (!ptr->on_key(k)) {
			if (dynamic_cast<ui::control *>(ptr) == root) {
				on_key(k);
				if (key_postlistener)
					key_postlistener(k);
				return this;
			}

			if (ui::child *cptr = dynamic_cast<ui::child *>(ptr)) {
				ptr = cptr->get_parent();
			} else {
				if (key_postlistener)
					key_postlistener(k);
				return nullptr;
			}
		}

		if (key_postlistener)
			key_postlistener(k);

		return ptr;
	}

	void terminal::send_mouse(const mouse_report &report) {
		if (report.mods == kmod::shift) {
			mouse(mouse_mode::none);
		}

		ui::control *ctrl = child_at_offset(report.x, report.y);
		if (ctrl == nullptr) {
			DBG(report.str() << " nullptr"_d);
		} else {
			DBG(report.str() << " " << ctrl->get_id());
		}
	}

	bool terminal::on_key(const key &k) {
		if (k == kmod::ctrl) {
			switch (k.type) {
				case ktype::l:
					redraw();
					break;
				case ktype::y: debug_tree(); break;
				default:
					return false;
			}

			return true;
		}

		return false;
	}

	void terminal::start_input() {
		input_thread = std::thread(&terminal::work_input, this);
	}

	void terminal::join() {
		if (input_thread.joinable())
			input_thread.join();
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

	bool terminal::add_child(ui::control *) {
		return false;
	}

	bool terminal::has_focus(const ui::control *ctrl) const {
		return focused == ctrl;
	}

	position terminal::get_position() const {
		return {0, 0, get_cols(), get_rows()};
	}

	ui::control * terminal::child_at_offset(int x, int y) const {
		ui::container *cont = dynamic_cast<container *>(root);
		while (cont != nullptr) {
			ui::control *ctrl = cont->child_at_offset(x, y);
			if (ctrl == nullptr)
				return nullptr;
			cont = dynamic_cast<container *>(ctrl);
			if (cont == nullptr)
				return ctrl;
		}

		return nullptr;
	}

	void terminal::jump_to_focused() {
		if (focused)
			focused->jump_focus();
	}

	void terminal::jump(int x, int y) {
		std::unique_lock uniq(output_mutex);
		out_stream.jump(x, y);
	}

	void terminal::mouse(mouse_mode mode) {
		std::unique_lock uniq(output_mutex);
		if (mode == mouse_mode::none) {
			if (mmode != mode) {
				DBG("\\e[?" << std::to_string(int(mmode)) << ";1006l");
				out_stream << "\e[?" << std::to_string(int(mmode)) << ";1006l";
				mmode = mode;
			}

			return;
		}

		if (mode != mmode) {
			if (mmode != mouse_mode::none) {
				out_stream << "\e[?" << std::to_string(int(mmode)) << "l";
				DBG("\\e[?" << std::to_string(int(mmode)) << "l");
			}

			DBG("\\e[?" << std::to_string(int(mode)) << ";1006h");
			out_stream << "\e[?" << std::to_string(int(mode)) << ";1006h";
			mmode = mode;
		}
	}

	void terminal::vscroll(int rows) {
		std::unique_lock uniq(output_mutex);
		if (0 < rows) {
			out_stream.scroll_down(rows);
		} else if (rows < 0) {
			out_stream.scroll_up(-rows);
		}
	}

	void terminal::hmargins(size_t left, size_t right) {
		std::unique_lock uniq(output_mutex);
		out_stream.hmargins(left, right);
	}

	void terminal::hmargins() {
		std::unique_lock uniq(output_mutex);
		out_stream.hmargins();
	}

	void terminal::vmargins(size_t top, size_t bottom) {
		std::unique_lock uniq(output_mutex);
		out_stream.vmargins(top, bottom);
	}

	void terminal::vmargins() {
		std::unique_lock uniq(output_mutex);
		out_stream.vmargins();
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
		std::unique_lock uniq(output_mutex);
		out_stream.enable_hmargins();
	}

	void terminal::disable_hmargins() {
		std::unique_lock uniq(output_mutex);
		out_stream.disable_hmargins();
	}

	void terminal::set_origin() {
		std::unique_lock uniq(output_mutex);
		out_stream.set_origin();
	}

	void terminal::reset_origin() {
		std::unique_lock uniq(output_mutex);
		out_stream.reset_origin();
	}

	std::unique_lock<std::recursive_mutex> terminal::lock_render() {
		return std::unique_lock(render_mutex);
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
				// We can't tell the difference between an actual press of the escape key and the beginning of a CSI.
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

				const char back = buffer.back();
				if (back == 'M' || back == 'm') {
					if (buffer.front() != '<') {
						DBG("Unrecognized sequence: \"" << buffer << "\"");
						throw std::invalid_argument("Unrecognized sequence");
					}

					mouse_report report {buffer};

					if (report.action == mouse_action::down) {
						dragging = true;
						drag_button = report.button;
					} else if (report.action == mouse_action::up) {
						dragging = false;
					}

					if (dragging && report.action == mouse_action::move) {
						report.action = mouse_action::drag;
						report.button = drag_button;
					}

					send_mouse(report);

					k = ktype::mouse;
					return *this;
				}

				const csi parsed = buffer;

				// Sometimes, get_key() returns keys with modifiers already set. For example, ^[Z represents shift+tab.
				// If these modifiers are already set, then modifiers weren't specified the CSI u way and we shouldn't
				// change them.
				k = parsed.get_key();
				if (k.mods.none())
					k = key(k.type, modset((parsed.second - 1) & 7));

				return *this;
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

	void terminal::debug_tree() {
		ansi::ansistream &dbg = haunted::dbgstream;
		dbg << "terminal"_b << ansi::endl;

		if (root) {
			std::deque<std::pair<int, ui::control *>> queue {{1, root}};

			int depth;
			ui::control *ctrl;

			while (!queue.empty()) {
				std::tie(depth, ctrl) = queue.back();
				queue.pop_back();

				const haunted::position &pos = ctrl->get_position();
				int width = pos.width, height = pos.height, top = pos.top, left = pos.left;

				dbg << ansi::color::gray << std::string(depth * 2, ' ') << ansi::action::reset
					<< ctrl->get_id();
				dbg.jump(25, -1).save()        << "("_d << left << ","_d;
				dbg.restore().right(6)         << top << ") "_d;
				dbg.restore().right(10).save() << width;
				dbg.restore().right(3)         << " × "_d << height << ansi::endl;
				if (ui::container *cont = dynamic_cast<ui::container *>(ctrl)) {
					for (ui::control *child: cont->get_children())
						queue.push_back({depth + 1, child});
				}
			}
		}
	}
}
