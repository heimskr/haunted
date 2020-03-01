#include <sstream>

#include "haunted/core/terminal.h"
#include "haunted/core/util.h"
#include "haunted/ui/control.h"

#include "lib/formicine/ansi.h"

namespace Haunted::UI {
	Control::Control(container *parent_, Haunted::position pos_): child(parent_), term(nullptr), pos(pos_) {
		if (parent_ != nullptr)
			term = parent_->get_terminal();
	}

	Control::Control(container *parent_, terminal *term_): child(parent_), term(term_) {}

	Control::Control(container *parent_): Control(parent_, parent_ == nullptr? nullptr : parent_->get_terminal()) {}

	Control::~Control() = default;


// Protected instance methods


	bool Control::try_margins(std::function<void()> fn) {
		const bool should_reset_margins = !in_margins;

		if (should_reset_margins)
			set_margins();

		fn();

		if (should_reset_margins)
			reset_margins();

		return should_reset_margins;
	}


// Public instance methods


	void Control::resize(const Haunted::position &new_pos) {
		// It's up to the caller of resize() to also call draw().
		pos = new_pos;
	}

	std::string Control::get_id(bool pad) const {
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

	bool Control::can_draw() const {
		return parent != nullptr && term != nullptr && 0 <= pos.left && 0 <= pos.top;
	}

	void Control::resize() {
		resize(pos);
	}

	void Control::move(int left, int top) {
		pos.left = left;
		pos.top = top;
	}

	void Control::focus() {
		if (term)
			term->focus(this);
	}

	void Control::set_parent(container *parent_) {
		child::set_parent(parent_);
		if (parent_ != nullptr) {
			terminal *parent_term = parent_->get_terminal();
			if (parent_term != nullptr)
				set_terminal(parent_term);
		}
	}

	void Control::jump() {
		pos.jump();
	}

	void Control::jump_focus() {
		jump();
	}

	void Control::clear_rect() {
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

	void Control::flush() {
		if (term != nullptr)
			term->flush();
	}

	bool Control::has_focus() const {
		return term && term->has_focus(this);
	}

	bool Control::at_right() const {
		return term != nullptr && pos.left + pos.width == term->get_cols();
	}

	bool Control::at_left() const {
		return pos.left == 0;
	}

	void Control::set_margins() {
		if (term != nullptr) {
			term->enable_hmargins();
			term->margins(pos.top, pos.bottom(), pos.left, pos.right());
			term->set_origin();
			in_margins = true;
		}
	}

	void Control::set_hmargins() {
		if (term != nullptr) {
			term->enable_hmargins();
			term->hmargins(pos.left, pos.right());
			term->set_origin();
		}
	}

	void Control::reset_margins() {
		if (term != nullptr) {
			term->reset_origin();
			term->margins();
			term->disable_hmargins();
			in_margins = false;
		}
	}

	ssize_t Control::get_index() const {
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

	void swap(Control &left, Control &right) {
		swap(static_cast<child &>(left), static_cast<child &>(right));
		std::swap(left.term, right.term);
		std::swap(left.name, right.name);
		std::swap(left.pos,  right.pos);
		std::swap(left.in_margins, right.in_margins);
	}
}
