#include "core/key.h"
#include "ui/keyhandler.h"

namespace haunted::ui {
	/**
	 * Handles a key input.
	 * If the function returns false, that means the key handler decided not to handle the key.
	 * If the key handler is a control or a container, then its parent's on_key method will be called.
	 * This can continue until the root is reached.
	 */
	bool keyhandler::on_key(key &) {
		return false;
	}
}
