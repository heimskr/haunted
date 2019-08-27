#include "ui/container.h"
#include "ui/control.h"
#include "core/util.h"
#include "core/terminal.h"

namespace haunted::ui {
	container::~container()
	//  = default; /*
	{
		if (control *ctrl = dynamic_cast<control *>(this)) {
			DBG("~container(" << ctrl->get_id() << "): size() == " << children.size());
		} else if (terminal *t = dynamic_cast<terminal *>(this)) {
			DBG("~container(terminal *" << this << "): size() == " << children.size());
		} else {
			DBG("~container(" << util::demangle_object(this) << " " << this << "): size() == " << children.size());
		}


		for (control *child_control: children) {
			std::string prefix;

			DBG(prefix << "Deleting " << (child_control == nullptr? "null" : child_control->get_id()));
			delete child_control;
		}
	}//*/

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

	bool container::request_resize(control *, size_t, size_t) {
		return false;
	}

	void container::redraw() {
		for (control *child: children)
			child->draw();
	}
}
