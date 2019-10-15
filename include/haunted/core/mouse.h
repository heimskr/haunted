#ifndef HAUNTED_CORE_MOUSE_H_
#define HAUNTED_CORE_MOUSE_H_

#include "haunted/core/key.h"

namespace haunted {
	enum class mouse_mode: int {none = 0, basic = 9, normal = 1000, highlight = 1001, motion = 1002, any = 1003};
	enum class mouse_action: char {move, down, up, drag, scrollup, scrolldown};
	enum class mouse_button {left, right};

	class mouse_report {
		private:
			char final_char;

		public:
			mouse_action action;
			mouse_button button;
			modset mods;
			long x, y; // zero-based.

			mouse_report(long, long, long);

			/** Parses a mouse report from a raw mouse report, which is expected to be /^<\d+;\d+;\d+[Mm]$/. */
			mouse_report(const std::string &);

			/** Splits a first parameter and a final character into an action, button and modset. */
			static void decode_type(long, char, mouse_action &, mouse_button &, modset &);
	};
}

#endif
