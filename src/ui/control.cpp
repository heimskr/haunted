#include "ui/control.h"

namespace haunted::ui {
	int control::max_children() const {
		return -1;
	}

	int control::child_count() const {
		return children.size();
	}

	control * control::operator[](size_t index) {
		return index < children.size()? children.at(index) : nullptr;
	}
}
