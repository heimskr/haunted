#include "haunted/ui/boxes/orientedbox.h"

namespace Haunted::UI::Boxes {
	int orientedbox::get_size() const {
		return orientation == box_orientation::horizontal? pos.width : pos.height;
	}

	void orientedbox::set_orientation(box_orientation new_orientation) {
		if (orientation != new_orientation) {
			orientation = new_orientation;
			resize();
		}
	}
}
