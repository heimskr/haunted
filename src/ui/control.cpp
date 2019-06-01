#include "ui/control.h"

namespace haunted::ui {
	control::~control() = default;

	int control::max_children() const {
		return -1;
	}

	int control::child_count() const {
		return children.size();
	}

	void control::resize(const haunted::position &new_pos) {
		pos = new_pos;
		draw();
	}

	control * control::operator[](size_t index) {
		return index < children.size()? children.at(index) : nullptr;
	}
}
