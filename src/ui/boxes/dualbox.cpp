#include "haunted/ui/boxes/dualbox.h"

namespace Haunted::UI::Boxes {
	void dualbox::resize(const position &new_pos) {
		if (orientation == box_orientation::horizontal) {
			if (Control *left = (*this)[0])
				left->resize({new_pos.left, new_pos.top, size_one(), new_pos.height});
			
			if (Control *right = (*this)[1])
				right->resize({new_pos.left + size_one(), new_pos.top, size_two(), new_pos.height});
		} else {
			if (Control *top = (*this)[0])
				top->resize({new_pos.left, new_pos.top, new_pos.width, size_one()});
			
			if (Control *bottom = (*this)[0])
				bottom->resize({new_pos.left, new_pos.top + size_one(), new_pos.width, size_two()});
		}
	}

	int dualbox::max_children() const {
		return 2;
	}
}
