#include "ui/control.h"

namespace haunted::ui {
	control::~control() = default;

	void control::resize(const haunted::position &new_pos) {
		// It's up to the caller of resize() to also call draw().
		pos = new_pos;
	}
}
