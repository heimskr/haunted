#include "formicine/ansi.h"

#include "core/terminal.h"
#include "ui/control.h"

namespace haunted::ui {
	control::~control() = default;

	void control::resize(const haunted::position &new_pos) {
		// It's up to the caller of resize() to also call draw().
		pos = new_pos;
	}

	bool control::can_draw() const {
		return parent != nullptr;
	}

	void control::resize() {
		resize(pos);
	}

	void control::move(int left, int top) {
		pos.left = left;
		pos.top = top;
	}

	void control::focus() {
		term->focus(this);
	}

	void control::set_parent(container *parent_) {
		child::set_parent(parent_);
		if (parent_ != nullptr)
			set_terminal(parent_->get_terminal());
	}

	terminal * control::get_terminal() const {
		return term;
	}

	void control::set_terminal(terminal *term_) {
		term = term_;
	}

	haunted::position control::get_position() const {
		return pos;
	}

	void control::jump() {
		pos.jump();
	}

	void control::clear_rect() {
		if (term == nullptr)
			return;

		// There are four different ways to clear text on a row: erase the entire row, erase everything to the left of
		// the cursor, erase everything to the right of the cursor and simply writing spaces. The first is ideal, the
		// next two are okay and the last is the worst option. Which option we can use depends on the left-offset and
		// width of the control.
		term->out_stream.save();
		if (pos.left == 0) {
			// If we're at the left edge of the screen, we can clear-line if the width is full, or clear-left otherwise.
			if (pos.width == term->get_cols()) {
				for (int i = 0; i < pos.height; ++i) {
					term->jump(0, pos.top + i);
					term->clear_line();
				}
			} else {
				for (int i = 0; i < pos.height; ++i) {
					term->out_stream.jump(pos.width, pos.top + i);
					term->out_stream.clear_left();
				}
			}
		} else if (pos.left + pos.width == term->get_cols()) {
			// If we're at the right edge of the screen, we can clear-right.
			for (int i = 0; i < pos.height; ++i) {
				term->out_stream.jump(pos.left, pos.top + i);
				term->out_stream.clear_right();
			}
		} else {
			// If the control doesn't reach either end of the screen, we have to print a bunch of spaces.
			std::string spaces(pos.width, ' ');
			for (int i = 0; i < pos.height; ++i) {
				term->out_stream.jump(pos.left, pos.top + i);
				std::cout << spaces;
			}
		}

		term->out_stream.restore();
	}

	void control::flush() {
		if (term != nullptr)
			term->flush();
	}

	bool control::has_focus() const {
		return term->has_focus(this);
	}

	bool control::at_right() const {
		return term != nullptr && pos.left + pos.width == term->get_cols();
	}

	bool control::at_left() const {
		return pos.left == 0;
	}

	void control::set_margins() {
		if (term == nullptr) return;
		term->enable_hmargins();
		term->margins(pos.top, pos.bottom(), pos.left, pos.right());
		term->set_origin();
	}

	void control::reset_margins() {
		if (term == nullptr) return;
		term->reset_origin();
		term->margins();
	}
}
