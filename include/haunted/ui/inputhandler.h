#ifndef HAUNTED_UI_INPUTHANDLER_H_
#define HAUNTED_UI_INPUTHANDLER_H_

#include <functional>

#include "haunted/core/key.h"
#include "haunted/core/mouse.h"

namespace haunted::ui {
	/**
	 * An input handler is anything that can handle key presses or mouse events.
	 * This includes controls and containers.
	 */
	class inputhandler {
		using   keyhandler_function = std::function<bool(const key &)>;
		using mousehandler_function = std::function<bool(const mouse_report &)>;

		public:
			/** This is a key-handling function that can be changed during runtime to replace on_key. Like on_key, a
			 *  return value of `true` means the key was handled and a return value of `false` passes the key up the
			 *  hierarchy. */
			keyhandler_function key_fn;

			mousehandler_function mouse_fn;

			/** Handles a key input.
			 *  If the function returns false, that means the key handler decided not to handle the key.
			 *  If the key handler is a control or a container, then its parent's on_key method will be called.
			 *  This can continue until the root is reached. */
			virtual bool on_key(const key &);

			virtual bool on_mouse(const mouse_report &);
	};
}

#endif
