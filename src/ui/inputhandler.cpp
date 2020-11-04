#include "haunted/ui/InputHandler.h"
#include "haunted/ui/Control.h"

namespace Haunted::UI {
	bool InputHandler::onKey(const Key &k) {
		return keyFunction? keyFunction(k) : false;
	}

	bool InputHandler::onMouse(const MouseReport &m) {
		return mouseFunction? mouseFunction(m) : false;
	}
}
