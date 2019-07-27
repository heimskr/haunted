#include "core/fix.h"
#include "ui/container.h"
#include "ui/child.h"

namespace haunted::ui {
	container * child::get_parent() {
		return parent;
	}
}
