#include "haunted/ui/boxes/DualBox.h"

namespace Haunted::UI::Boxes {
	void DualBox::resize(const Position &new_pos) {
		if (orientation == BoxOrientation::Horizontal) {
			if (Control *left = (*this)[0])
				left->resize({new_pos.left, new_pos.top, sizeOne(), new_pos.height});
			
			if (Control *right = (*this)[1])
				right->resize({new_pos.left + sizeOne(), new_pos.top, sizeTwo(), new_pos.height});
		} else {
			if (Control *top = (*this)[0])
				top->resize({new_pos.left, new_pos.top, new_pos.width, sizeOne()});
			
			if (Control *bottom = (*this)[1])
				bottom->resize({new_pos.left, new_pos.top + sizeOne(), new_pos.width, sizeTwo()});
		}
	}
}
