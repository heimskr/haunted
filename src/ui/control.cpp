#include <sstream>

#include "formicine/ansi.h"
#include "core/terminal.h"
#include "core/util.h"
#include "ui/control.h"

namespace haunted::ui {
	control::~control() = default;

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
			ss << demangled.substr(demangled.find_last_of(':') + 1) << "\e[2m|\e[0;33m" << this;
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
		return parent != nullptr && term != nullptr;
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
		if (parent_ != nullptr) {
			terminal *parent_term = parent_->get_terminal();
			if (parent_term != nullptr)
				set_terminal(parent_term);
		}
	}

	void control::jump() {
		pos.jump();
	}

	void control::clear_rect() {
		if (term != nullptr) {
			term->out_stream.save();
			set_margins();
			term->out_stream.clear();
			reset_margins();
			term->out_stream.restore();
		}
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
