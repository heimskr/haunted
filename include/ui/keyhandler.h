#ifndef HAUNTED_UI_KEYHANDLER_H_
#define HAUNTED_UI_KEYHANDLER_H_

#include <functional>

#include "core/key.h"

namespace haunted::ui {
	/**
	 * A key handler is anything that can handle key presses.
	 * This includes controls and containers.
	 */
	class keyhandler {
		using keyhandler_function = std::function<bool(const key &)>;

		public:
			keyhandler_function key_fn;

			virtual bool on_key(const key &);
	};
}

#endif
