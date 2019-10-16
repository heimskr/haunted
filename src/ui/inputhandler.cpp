#include "haunted/ui/inputhandler.h"
#include "haunted/ui/control.h"

namespace haunted::ui {
	bool inputhandler::on_key(const key &k) {
		return key_fn? key_fn(k) : false;
	}

	bool inputhandler::on_mouse(const mouse_report &m) {
		return mouse_fn? mouse_fn(m) : false;
	}
}
