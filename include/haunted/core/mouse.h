#ifndef HAUNTED_CORE_MOUSE_H_
#define HAUNTED_CORE_MOUSE_H_

#include "haunted/core/Key.h"

namespace Haunted {
	enum class MouseMode: int {None = 0, Basic = 9, Normal = 1000, Highlight = 1001, Motion = 1002, Any = 1003};
	enum class MouseAction: char {Move, Down, Up, Drag, ScrollUp, ScrollDown};
	enum class MouseButton {Left, Right};

	class MouseReport {
		private:
			char finalChar;

		public:
			MouseAction action;
			MouseButton button;
			ModSet mods;
			long x, y; // zero-based.

			MouseReport(long type, char fchar, long x, long y);

			/** Parses a mouse report from a raw mouse report, which is expected to be /^<\d+;\d+;\d+[Mm]$/. */
			MouseReport(const std::string &);

			/** Returns a string that describes the mouse report. */
			std::string str() const;

			/** Splits a first parameter and a final character into an action, button and modset. */
			static void decodeType(long, char, MouseAction &, MouseButton &, ModSet &);
	};
}

#endif
