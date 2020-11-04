#include "haunted/ui/ColoredControl.h"

namespace Haunted::UI {
	void ColoredControl::clearRect() {
		if (!canDraw())
			return;
		
		applyColors();
		Control::clearRect();
		uncolor();
	}
}
