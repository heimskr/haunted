#include "haunted/ui/boxes/OrientedBox.h"

namespace Haunted::UI::Boxes {
	int OrientedBox::getSize() const {
		return orientation == BoxOrientation::Horizontal? position.width : position.height;
	}

	void OrientedBox::setOrientation(BoxOrientation new_orientation) {
		if (orientation != new_orientation) {
			orientation = new_orientation;
			resize();
		}
	}
}
