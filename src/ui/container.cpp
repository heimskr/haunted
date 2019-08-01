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

	bool container::remove_child(child *child) {
		for (auto iter = children.begin(); iter != children.end(); ++iter) {
			if (*iter == child) {
				children.erase(iter);
				child->set_parent(nullptr);
				return true;
			}
		}

		return false;
	}

	int container::max_children() const {
		return -1;
	}

	int container::child_count() const {
		return children.size();
	}
}
