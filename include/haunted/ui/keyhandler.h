#ifndef HAUNTED_UI_KEYHANDLER_H_
#define HAUNTED_UI_KEYHANDLER_H_

#include <functional>

#include "haunted/core/key.h"

namespace haunted::ui {
	/**
	 * A key handler is anything that can handle key presses.
	 * This includes controls and containers.
	 */
	class keyhandler {
		using keyhandler_function = std::function<bool(const key &)>;

		public:
			/** This is a key-handling function that can be changed during runtime to replace on_key. Like on_key, a
			 *  return value of `true` means the key was handled and a return value of `false` passes the key up the
			 *  hierarchy. */
			keyhandler_function key_fn;

			virtual bool on_key(const key &);
	};
}

#endif
