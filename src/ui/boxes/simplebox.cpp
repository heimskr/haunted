#include "ui/boxes/simplebox.h"

namespace haunted::ui::boxes {
	void simplebox::resize(const position &pos) {
		control::resize(pos);
		if (!children.empty())
			children.at(0)->move(pos.left, pos.top);
	}

	void simplebox::move(int left, int top) {
		control::move(left, top);
		if (!children.empty())
			children.at(0)->move(left, top);
	}

	void simplebox::draw() {
		if (!children.empty())
			children.at(0)->draw();
	}

	haunted::terminal * simplebox::get_terminal() {
		return term;
	}
}
