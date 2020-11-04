#ifndef HAUNTED_UI_COLORATION_H_
#define HAUNTED_UI_COLORATION_H_

#include <mutex>

#include "lib/formicine/ansi.h"

namespace Haunted::UI {
	/**
	 * Class for handling terminal colors. Each Haunted::terminal keeps one of these.
	 * 
	 */
	class Coloration {
		private:
			ansi::ansistream *outStream;
			std::mutex *mutex;
			ansi::color lastForeground = ansi::color::normal;
			ansi::color lastBackground = ansi::color::normal;

			std::unique_lock<std::mutex> getLock() { return std::unique_lock(*mutex); }

		public:
			Coloration(ansi::ansistream *out_stream, std::mutex *mutex_): outStream(out_stream), mutex(mutex_) {}

			/** Attempts to set the foreground. Returns whether the given foreground is different from the last one. */
			bool setForeground(ansi::color);

			/** Attempts to set the background. Returns whether the given background is different from the last one. */
			bool setBackground(ansi::color);

			/** Attempts to set both the foreground and the background. Returns whether any change occurred. */
			bool setBoth(ansi::color foreground, ansi::color background);

			/** Sets the terminal's colors to the stored foreground and background colors. */
			void apply();

			/** Attempts to reset the colors to the terminal's defaults. Returns whether any change occurred. */
			bool reset();

			/** Prints the foreground and background information to the debug log. */
			void debug();

			ansi::color getForeground() const { return lastForeground; }
			ansi::color getBackground() const { return lastBackground; }
	};
}

#endif
