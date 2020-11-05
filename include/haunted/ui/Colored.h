#ifndef HAUNTED_UI_COLORED_H_
#define HAUNTED_UI_COLORED_H_

#include "haunted/ui/Control.h"

#include "lib/formicine/ansi.h"

namespace Haunted::UI {
	/**
	 * Represents something that can be colored.
	 */
	class Colored {
		private:
			ansi::color background, foreground;

			/** If the specified color type is "normal" for this, this function searches all ancestors until it finds
			 *  one with a color of the same type and returns it. Otherwise, this returns the specified color. */
			ansi::color findColor(ansi::color_type) const;

		public:
			bool inheritForeground, inheritBackground;

			Colored(const Colored &) = delete;
			Colored & operator=(const Colored &) = delete;

			Colored(ansi::color foreground = ansi::color::normal, ansi::color background = ansi::color::normal,
			bool inherit_fg = false, bool inherit_bg = false):
				background(background), foreground(foreground), inheritForeground(inherit_fg),
				inheritBackground(inherit_bg) {}

			virtual ~Colored() = 0;

			ansi::color getForeground() const { return foreground; }
			ansi::color getBackground() const { return background; }
			bool setForeground(ansi::color);
			bool setBackground(ansi::color);
			/** Sets the foreground and background colors of the control. */
			bool setColors(ansi::color foreground_, ansi::color background_);
			bool setColors(ansi::color foreground_, ansi::color background_, bool inherit_fg, bool inherit_bg);
			bool setInherit(bool inherit_fg, bool inherit_bg);
			/** Applies the control's colors to the terminal. */
			Colored & applyColors();
			/** Applies the control's colors to the terminal if they are not the currently active colors. */
			Colored & tryColors(bool find = false);
			/** Resets the terminal's colors. */
			Colored & uncolor();
			/** Propagate the control's colors to its children. */
			bool propagate(ansi::color_type);

			virtual void draw();
			virtual void focus();

			virtual Container * getParent() const = 0;
			virtual Terminal * getTerminal() = 0;

			friend void swap(Colored &left, Colored &right);
	};
}

#endif
