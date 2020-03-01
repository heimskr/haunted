#include "haunted/core/terminal.h"
#include "haunted/ui/label.h"

namespace Haunted::UI {

	label::label(container *parent_, const position &pos_, const std::string &text_, bool autoresize_,
	const std::string &cutoff): control(parent_, pos_), text(text_), cutoff(cutoff), autoresize(autoresize_) {
		if (parent_ != nullptr)
			parent_->add_child(this);
	}


// Public instance methods


	void label::clear() {
		set_text("");
	}

	void label::set_text(const std::string &text_) {
		if (text != text_) {
			text = text_;
			draw();
		}
	}

	void label::set_autoresize(bool autoresize_) {
		autoresize = autoresize_;
		if (autoresize_ && static_cast<size_t>(pos.width) < length() && parent != nullptr)
			parent->request_resize(this, length(), pos.height);
	}

	void label::draw() { // TODO: proper Unicode :~)
		if (!can_draw())
			return;

		auto lock = term->lock_render();
		colored::draw();
		jump();

		size_t tlen = ansi::strip(text).length(), clen = ansi::strip(cutoff).length(), width = pos.width;

		if (tlen == width) {
			*term << text;
		} else if (tlen < width) {
			*term << text << std::string(width - tlen, ' ');
		} else if (cutoff.empty()) {
			*term << ansi::substr(text, 0, width);
		} else if (clen == width) {
			*term << cutoff;
		} else if (width < clen) {
			*term << ansi::substr(cutoff, 0, width);
		} else {
			*term << ansi::substr(text, 0, width - clen) << cutoff;
		}

		term->reset_colors();
		term->flush();
		term->jump_to_focused();
	}

	bool label::can_draw() const {
		return parent != nullptr && term != nullptr && !term->suppress_output;
	}

	std::ostream & operator<<(std::ostream &os, const label &input) {
		os << std::string(input);
		return os;
	}
}
