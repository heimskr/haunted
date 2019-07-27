#include "ui/boxes/simplebox.h"

namespace haunted::ui::boxes {
	void simplebox::resize(const position &pos) {
		control::resize(pos);
		children.at(0)->move(pos.left, pos.top);
	}

	void simplebox::move(int left, int top) {
		control::move(left, top);
		children.at(0)->move(left, top);
	}

	void simplebox::draw() {
		children.at(0)->draw();
	}

	haunted::terminal * simplebox::get_term() {
		return term;
	}
}
