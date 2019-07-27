#include "core/fix.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted::ui {
	container::~container() = default;	

	control * container::operator[](size_t index) {
		return index < children.size()? children.at(index) : nullptr;
	}

	bool container::add_child(control *child) {
		children.push_back(child);
		return true;
	}

	int container::max_children() const {
		return -1;
	}

	int container::child_count() const {
		return children.size();
	}
}
