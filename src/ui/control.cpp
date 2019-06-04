#include "lib/ansi.h"
#include "ui/control.h"

namespace haunted::ui {
	control::~control() = default;

	void control::resize(const haunted::position &new_pos) {
		// It's up to the caller of resize() to also call draw().
		pos = new_pos;
	}

	/**
	 * Handles a key input.
	 * If the function returns false, the parent's on_key method will be called.
	 * This can continue until the root is reached.
	 */
	bool control::on_key(key &) {
		return false;
	}

	/** Moves the cursor to the top-left corner of the control. */
	void control::jump() {
		ansi::jump(pos.top + 1, pos.left + 1);
	}
}
