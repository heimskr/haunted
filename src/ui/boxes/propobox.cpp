#include <stdexcept>

#include "core/terminal.h"
#include "ui/boxes/dualbox.h"
#include "ui/boxes/propobox.h"

namespace haunted::ui::boxes {
	propobox::propobox(container *parent_, const position &pos_, double ratio_, box_orientation orientation_):
	dualbox(parent_, pos_, orientation_), ratio(ratio_) {
		if (parent_)
			parent_->add_child(this);

		if (ratio < 0)
			throw std::domain_error("Box ratio cannot be negative");
	}

	propobox::propobox(container *parent_, double ratio_, box_orientation orientation_, control *one, control *two,
	const position &pos_): propobox(parent_, pos_, ratio_, orientation_) {
		for (control *ctrl: {one, two}) {
			ctrl->set_parent(this);
			ctrl->set_terminal(control::get_terminal());
			children.push_back(ctrl);
		}
	}

	void propobox::set_ratio(const double ratio_) {
		if (ratio != ratio_) {
			ratio = ratio_;
			draw();
		}
	}

	void propobox::draw() {
		if (!can_draw())
			return;

		colored::draw();

		auto lock = term->lock_render();
		for (control *child: children)
			child->draw();
	}

	int propobox::size_one() const {
		return get_size() - size_two();
	}

	int propobox::size_two() const {
		return get_size() / (1.0 + ratio);
	}

	int propobox::get_size() const {
		return orientation == box_orientation::horizontal? pos.width : pos.height;
	}
}
