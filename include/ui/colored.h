#ifndef HAUNTED_UI_COLORED_H_
#define HAUNTED_UI_COLORED_H_

#include "formicine/ansi.h"
#include "ui/control.h"

namespace haunted::ui {
	/**
	 * Represents a control that can be colored.
	 */
	class colored: public virtual control {
		private:
			ansi::color background, foreground;

			/** If the specified color type is "normal" for this control, this function searches the control's parents
			 *  until it finds one with a color of the same type and returns it. Otherwise, this returns the control's
			 *  specified color. */
			ansi::color find_color(ansi::color_type) const;

		public:
			bool inherit_foreground, inherit_background;

			colored(ansi::color foreground, ansi::color background, bool inherit_fg = false, bool inherit_bg = false):
				child(nullptr), background(background), foreground(foreground), inherit_foreground(inherit_fg),
				inherit_background(inherit_bg) {}
			colored(ansi::color foreground): colored(foreground, ansi::color::normal) {}
			colored(): colored(ansi::color::normal) {}

			ansi::color get_foreground() const { return foreground; }
			ansi::color get_background() const { return background; }
			bool set_foreground(ansi::color);
			bool set_background(ansi::color);
			/** Sets the foreground and background colors of the control. */
			bool set_colors(ansi::color foreground_, ansi::color background_);
			bool set_colors(ansi::color foreground_, ansi::color background_, bool inherit_fg, bool inherit_bg);
			bool set_inherit(bool inherit_fg, bool inherit_bg);
			/** Applies the control's colors to the terminal. */
			colored & apply_colors();
			/** Applies the control's colors to the terminal if they are not the currently active colors. */
			colored & try_colors(bool find = false);
			/** Resets the terminal's colors. */
			colored & uncolor();
			/** Propagate the control's colors to its children. */
			bool propagate(ansi::color_type);

			virtual void draw() override;
			virtual void focus() override;
	};
}

#endif
