#ifndef HAUNTED_UI_KEYHANDLER_H_
#define HAUNTED_UI_KEYHANDLER_H_

#include "core/key.h"

namespace haunted::ui {
	/**
	 * A key handler is anything that can handle key presses.
	 * This includes controls and containers.
	 */
	class keyhandler {
		public:
			virtual bool on_key(key);
	};
}

#endif
