#include <stdexcept>

#include "haunted/core/terminal.h"
#include "haunted/ui/boxes/dualbox.h"
#include "haunted/ui/boxes/propobox.h"

namespace Haunted::UI::Boxes {
	propobox::propobox(container *parent_, const position &pos_, double ratio_, box_orientation orientation_):
	dualbox(parent_, pos_, orientation_), ratio(ratio_) {
		if (parent_)
			parent_->add_child(this);

		if (ratio < 0)
			throw std::domain_error("Box ratio cannot be negative");
	}

	propobox::propobox(container *parent_, double ratio_, box_orientation orientation_, Control *one, Control *two,
	const position &pos_): propobox(parent_, pos_, ratio_, orientation_) {
		for (Control *ctrl: {one, two}) {
			ctrl->set_parent(this);
			ctrl->set_terminal(term);
			children.push_back(ctrl);
		}
	}

	void propobox::set_ratio(const double ratio_) {
		if (ratio != ratio_) {
			ratio = ratio_;
			resize();
		}
	}

	void propobox::resize(const position &new_pos) {
		Control::resize(new_pos);

		if (children.size() == 0) {
			return;
		} else if (children.size() == 1) {
			children[0]->resize(new_pos);
		} else if (children.size() == 2) {
			if (orientation == box_orientation::horizontal) {
				children[0]->resize({pos.left, pos.top, size_one(), pos.height});
				children[1]->resize({pos.left + size_one(), pos.top, size_two(), pos.height});
			} else if (orientation == box_orientation::vertical) {
				children[0]->resize({pos.left, pos.top, pos.width, size_one()});
				children[1]->resize({pos.left, pos.top + size_one(), pos.width, size_two()});
			} else throw std::runtime_error("Unknown orientation: " + std::to_string(static_cast<int>(orientation)));
		} else throw std::runtime_error("Invalid number of children for propobox: " + std::to_string(children.size()));

		redraw();
	}

	void propobox::draw() {
		if (!can_draw())
			return;

		colored::draw();

		auto lock = term->lock_render();
		for (Control *child: children)
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
