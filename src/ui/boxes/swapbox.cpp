#include "ui/boxes/swapbox.h"

namespace haunted::ui::boxes {
	swapbox::swapbox(container *parent_, const position &pos_, std::initializer_list<control *> children_):
	box(parent_, pos_) {
		if (parent_)
			parent_->add_child(this);

		for (control *ctrl: children_) {
			ctrl->set_parent(nullptr);
			children.push_back(ctrl);
		}

		if (children_.size() != 0) {
			active = *children_.begin();
			active->set_parent(this);
			active->set_terminal(term);
		}
	}


// Public instance methods


	void swapbox::set_active(control *new_active) {
		if (new_active == active)
			return;

		if (new_active == nullptr) {
			active->set_terminal(nullptr);
			active = nullptr;

			clear_rect();
		} else if (new_active != nullptr) {
			if (active != nullptr) {
				active->set_terminal(nullptr);
			}

			active = new_active;
			active->set_terminal(term);
			active->draw();
		}

		if (std::find(children.begin(), children.end(), new_active) == children.end())
			children.push_back(new_active);
	}

	void swapbox::resize(const position &new_pos) {
		control::resize(new_pos);
		if (active)
			active->resize(new_pos);
	}

	void swapbox::draw() {
		if (!can_draw())
			return;

		if (active)
			active->draw();
		else
			clear_rect();
	}
}
