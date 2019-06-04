#include "lib/ansi.h"
#include "ui/control.h"

namespace haunted::ui {
	control::~control() = default;

	void control::resize(const haunted::position &new_pos) {
		// It's up to the caller of resize() to also call draw().
		pos = new_pos;
	}

	/** Moves the cursor to the top-left corner of the control. */
	void control::jump() {
		ansi::jump(pos.top + 1, pos.left + 1);
	}

	container * control::get_parent() {
		return parent;
	}
}
