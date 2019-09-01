#include "ui/boxes/swapbox.h"

namespace haunted::ui::boxes {
	swapbox::swapbox(container *parent_, const position &pos_, std::initializer_list<control *> controls_):
	box(parent_, pos_), controls(controls_) {
		if (parent_)
			parent_->add_child(this);

		for (control *ctrl: controls_) {
			ctrl->set_parent(nullptr);
		}
	}

	void swapbox::resize(const position &new_pos) {
		control::resize(new_pos);
		if (active)
			active->resize(new_pos);
	}

	void swapbox::draw() {
		if (active)
			active->draw();
		else
			clear_rect();
	}
}
