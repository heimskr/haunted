#include "haunted/ui/colored_control.h"

namespace Haunted::UI {
	void colored_control::clear_rect() {
		if (!can_draw())
			return;
		
		apply_colors();
		control::clear_rect();
		uncolor();
	}
}
