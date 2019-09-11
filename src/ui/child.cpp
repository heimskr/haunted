#include "ui/container.h"
#include "ui/child.h"

namespace haunted::ui {
	child::~child() = default;

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

	void swap(child &left, child &right) {
		std::swap(left.parent, right.parent);
	}
}
