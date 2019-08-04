#include "core/terminal.h"
#include "ui/label.h"

namespace haunted::ui {

	label::label(container *parent, position pos, const std::string &text, bool autoresize, const std::string &cutoff):
	control(parent, pos), text(text), cutoff(cutoff), autoresize(autoresize) {
		if (parent != nullptr)
			parent->add_child(this);
	}


// Public instance methods


	void label::set_text(const std::string &text_) {
		bool request_resize = text.length() != text_.length();
		text = text_;

		if (request_resize && parent != nullptr)
			parent->request_resize(this, text_.length(), pos.height);

		draw();
	}

	void label::set_autoresize(bool autoresize_) {
		autoresize = autoresize_;
		if (autoresize_ && pos.width < length() && parent != nullptr)
			parent->request_resize(this, length(), pos.height);
	}

	void label::draw() { // TODO: proper Unicode :~)
		if (!can_draw())
			return;

		colored::draw();

		auto lock = term->lock_render();
		jump();

		size_t tlen = text.length(), clen = cutoff.length(), width = pos.width;

		if (tlen == width) {
			*term << text;
		} else if (tlen < width) {
			*term << text << std::string(width - tlen, ' ');
		} else if (cutoff.empty()) {
			*term << text.substr(0, width);
		} else if (clen == width) {
			*term << cutoff;
		} else if (width < clen) {
			*term << cutoff.substr(0, width);
		} else {
			*term << text.substr(0, width - clen) << cutoff;
		}
	}

	bool label::can_draw() const {
		return parent != nullptr && term != nullptr && !term->suppress_output;
	}

	std::ostream & operator<<(std::ostream &os, const label &input) {
		os << std::string(input);
		return os;
	}
}
