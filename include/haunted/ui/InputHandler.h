#ifndef HAUNTED_UI_INPUTHANDLER_H_
#define HAUNTED_UI_INPUTHANDLER_H_

#include <functional>

#include "haunted/core/Key.h"
#include "haunted/core/Mouse.h"

namespace Haunted::UI {
	/**
	 * An input handler is anything that can handle key presses or mouse events.
	 * This includes controls and containers.
	 */
	class InputHandler {
		using   KeyHandler_f = std::function<bool(const Key &)>;
		using MouseHandler_f = std::function<bool(const MouseReport &)>;

		public:
			/** This is a key-handling function that can be changed during runtime to replace on_key. Like on_key, a
			 *  return value of `true` means the key was handled and a return value of `false` passes the key up the
			 *  hierarchy. */
			KeyHandler_f keyFunction;

			MouseHandler_f mouseFunction;

			/** Handles a key input.
			 *  If the function returns false, that means the key handler decided not to handle the key.
			 *  If the key handler is a control or a container, then its parent's on_key method will be called.
			 *  This can continue until the root is reached. */
			virtual bool onKey(const Key &);

			virtual bool onMouse(const MouseReport &);
	};
}

#endif
