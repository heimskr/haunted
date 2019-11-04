#include <sstream>

#include "haunted/core/terminal.h"
#include "haunted/core/util.h"
#include "haunted/ui/control.h"

#include "lib/formicine/ansi.h"

namespace haunted::ui {
	control::~control() = default;


// Protected instance methods


	bool control::try_margins(std::function<void()> fn) {
		const bool should_reset_margins = !in_margins;

		if (should_reset_margins)
			set_margins();

		fn();

		if (should_reset_margins)
			reset_margins();

		return should_reset_margins;
	}


// Public instance methods


	void control::resize(const haunted::position &new_pos) {
		// It's up to the caller of resize() to also call draw().
		pos = new_pos;
	}

	std::string control::get_id(bool pad) const {
		std::stringstream ss;
		if (name.empty()) {
			std::string demangled = util::demangle_object(*this);
			if (pad)
				ss << std::setw(10);
			ss << demangled.substr(demangled.find_last_of(':') + 1) << "\e[2m|\e[0;33m" << this << "\e[39m";
		} else {
			
			// ss << ansi::get_fg(static_cast<ansi::color>(reinterpret_cast<size_t>(this) & 0x10));
			ansi::color c = static_cast<ansi::color>((reinterpret_cast<size_t>(this) >> 6) & 15);
			if (c == ansi::color::black) {
				c = ansi::color::green;
			} else if (c == ansi::color::normal) {
				c = ansi::color::red;
			}

			ss << ansi::get_fg(c);
			if (pad)
				ss << std::setw(10);
			ss << name << ansi::get_fg(ansi::color::normal);
		}

		return ss.str();
	}

	bool control::can_draw() const {
		return parent != nullptr && term != nullptr && 0 <= pos.left && 0 <= pos.top;
	}

	void control::resize() {
		resize(pos);
	}

	void control::move(int left, int top) {
		pos.left = left;
		pos.top = top;
	}

	void control::focus() {
		if (term)
			term->focus(this);
	}

	void control::set_parent(container *parent_) {
		child::set_parent(parent_);
		if (parent_ != nullptr) {
			terminal *parent_term = parent_->get_terminal();
			if (parent_term != nullptr)
				set_terminal(parent_term);
		}
	}

	void control::jump() {
		pos.jump();
	}

	void control::jump_focus() {
		jump();
	}

	void control::clear_rect() {
		if (term == nullptr)
			return;

		try_margins([&, this]() {
			if (at_left() && at_right()) {
				// Galaxy brain trickery here. If this control is as wide as the entire terminal, we can vscroll the
				// contents into oblivion very efficiently.
				term->vscroll(pos.height);
			} else if (at_left()) {
				// If we're at the left, we can clear each line from the end of the line to the left edge of the screen.
				term->jump(pos.width - 1, 0);
				for (int i = 0; i < pos.height; ++i) {
					if (i)
						term->down();
					term->clear_left();
				}
			} else if (at_right()) {
				// If we're at the right, we can clear each line from the start of the line to the right edge.
				term->jump(pos.left, 0);
				for (int i = 0; i < pos.height; ++i) {
					if (i)
						term->down();
					term->clear_right();
				}
			} else {
				// If we're at neither edge, we have to print a total of width*height spaces. Very sad.
				std::string spaces(static_cast<size_t>(pos.width), ' ');
				for (int i = 0; i < pos.height; ++i) {
					term->jump(pos.left, i);
					*term << spaces;
				}
			}
		});
	}

	void control::flush() {
		if (term != nullptr)
			term->flush();
	}

	bool control::has_focus() const {
		return term && term->has_focus(this);
	}

	bool control::at_right() const {
		return term != nullptr && pos.left + pos.width == term->get_cols();
	}

	bool control::at_left() const {
		return pos.left == 0;
	}

	void control::set_margins() {
		if (term != nullptr) {
			term->enable_hmargins();
			term->margins(pos.top, pos.bottom(), pos.left, pos.right());
			term->set_origin();
			in_margins = true;
		}
	}

	void control::set_hmargins() {
		if (term != nullptr) {
			term->enable_hmargins();
			term->hmargins(pos.left, pos.right());
			term->set_origin();
		}
	}

	void control::reset_margins() {
		if (term != nullptr) {
			term->reset_origin();
			term->margins();
			term->disable_hmargins();
			in_margins = false;
		}
	}

	ssize_t control::get_index() const {
		if (parent != nullptr && !ignore_index) {
			ssize_t i = 0;
			for (auto iter = parent->begin(); iter != parent->end(); ++iter, ++i) {
				if (*iter == this)
					return i;

				if ((*iter)->ignore_index)
					--i;
			}
		}

		return -1;
	}

	void swap(control &left, control &right) {
		swap(static_cast<child &>(left), static_cast<child &>(right));
		std::swap(left.term, right.term);
		std::swap(left.name, right.name);
		std::swap(left.pos,  right.pos);
		std::swap(left.in_margins, right.in_margins);
	}
}
