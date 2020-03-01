#include "haunted/ui/container.h"
#include "haunted/ui/control.h"
#include "haunted/core/util.h"
#include "haunted/core/terminal.h"

namespace Haunted::UI {
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

	control * container::child_at_offset(int x, int y) const {
		const position pos = get_position();
		x += pos.left;
		y += pos.top;

		for (control *child: children) {
			const position &cpos = child->get_position();
			if ((cpos.left <= x) && (x <= cpos.right()) && (cpos.top <= y) && (y <= cpos.bottom()))
				return child;
		}

		return nullptr;
	}

	bool container::request_resize(control *, size_t, size_t) {
		return false;
	}

	void container::redraw() {
		for (control *child: children)
			child->draw();
	}
}
