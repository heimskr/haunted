#include "ui/container.h"
#include "ui/child.h"

namespace haunted::ui {
	container * child::get_parent() {
		return parent;
	}

	void child::set_parent(container *new_parent) {
		if (parent == new_parent)
			return;

		if (parent != nullptr)
			parent->remove_child(this);

		parent = new_parent;
	}
}
