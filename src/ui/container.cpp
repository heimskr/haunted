#include "ui/container.h"
#include "ui/control.h"
#include "core/util.h"
#include "core/terminal.h"

namespace haunted::ui {
	container::~container() {
		for (control *child: children)
			delete child;
	}

	control * container::operator[](size_t index) {
		return index < children.size()? children.at(index) : nullptr;
	}

	bool container::add_child(control *child) {
		children.push_back(child);
		return true;
	}

	bool container::remove_child(child *to_remove) {
		for (auto iter = children.begin(); iter != children.end(); ++iter) {
			if (*iter == to_remove) {
				children.erase(iter);
				to_remove->set_parent(nullptr);
				return true;
			}
		}

		return false;
	}

	bool container::request_resize(control *, size_t, size_t) {
		return false;
	}

	void container::redraw() {
		for (control *child: children)
			child->draw();
	}
}
