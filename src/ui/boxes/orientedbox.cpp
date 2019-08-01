#include "ui/boxes/orientedbox.h"

namespace haunted::ui::boxes {
	int orientedbox::get_size() const {
		return orientation == horizontal? pos.width : pos.height;
	}
}
