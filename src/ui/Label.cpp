#include "haunted/core/Terminal.h"
#include "haunted/ui/Label.h"

namespace Haunted::UI {

	Label::Label(Container *parent_, const Position &pos, const std::string &text_, bool autoresize_,
	const std::string &cutoff): Control(parent_, pos), text(text_), cutoff(cutoff), autoresize(autoresize_) {
		if (parent_)
			parent_->addChild(this);
	}


// Public instance methods


	void Label::clear() {
		setText("");
	}

	void Label::setText(const std::string &text_) {
		if (text != text_) {
			text = text_;
			draw();
		}
	}

	void Label::setAutoresize(bool autoresize_) {
		autoresize = autoresize_;
		if (autoresize_ && static_cast<size_t>(position.width) < length() && parent != nullptr)
			parent->requestResize(this, length(), position.height);
	}

	void Label::draw() { // TODO: proper Unicode :~)
		if (!canDraw())
			return;

		auto lock = terminal->lockRender();
		Colored::draw();
		jump();

		const size_t tlen = ansi::strip(text).length(), clen = ansi::strip(cutoff).length(), width = position.width;

		if (tlen == width) {
			*terminal << text;
		} else if (tlen < width) {
			*terminal << text << std::string(width - tlen, ' ');
		} else if (cutoff.empty()) {
			*terminal << ansi::substr(text, 0, width);
		} else if (clen == width) {
			*terminal << cutoff;
		} else if (width < clen) {
			*terminal << ansi::substr(cutoff, 0, width);
		} else {
			*terminal << ansi::substr(text, 0, width - clen) << cutoff;
		}

		terminal->resetColors();
		terminal->flush();
		terminal->jumpToFocused();
	}

	bool Label::canDraw() const {
		return parent != nullptr && terminal != nullptr && !terminal->suppressOutput;
	}
}

std::ostream & operator<<(std::ostream &os, const Haunted::UI::Label &label) {
	return os << std::string(label);
}
