#ifndef HAUNTED_UI_COLORATION_H_
#define HAUNTED_UI_COLORATION_H_

#include <mutex>

#include "lib/formicine/ansi.h"

namespace Haunted::UI {
	/**
	 * Class for handling terminal colors. Each Haunted::Terminal keeps one of these.
	 * 
	 */
	class Coloration {
		private:
			ansi::ansistream *out_stream;
			std::mutex *mux;
			ansi::color last_foreground = ansi::color::normal;
			ansi::color last_background = ansi::color::normal;

			std::unique_lock<std::mutex> get_lock() { return std::unique_lock(*mux); }

		public:
			Coloration(ansi::ansistream *out_stream_, std::mutex *mux_): out_stream(out_stream_), mux(mux_) {}

			/** Attempts to set the foreground. Returns whether the given foreground is different from the last one. */
			bool set_foreground(ansi::color);

			/** Attempts to set the background. Returns whether the given background is different from the last one. */
			bool set_background(ansi::color);

			/** Attempts to set both the foreground and the background. Returns whether any change occurred. */
			bool set_both(ansi::color foreground, ansi::color background);

			/** Sets the terminal's colors to the stored foreground and background colors. */
			void apply();

			/** Attempts to reset the colors to the terminal's defaults. Returns whether any change occurred. */
			bool reset();

			/** Prints the foreground and background information to the debug log. */
			void debug();

			ansi::color get_foreground() const { return last_foreground; }
			ansi::color get_background() const { return last_background; }
	};
}

#endif
