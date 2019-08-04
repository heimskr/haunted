#ifndef HAUNTED_UI_COLORED_H_
#define HAUNTED_UI_COLORED_H_

#include "lib/formicine/ansi.h"
#include "ui/control.h"

namespace haunted::ui {
	class colored: public virtual control {
		private:
			ansi::color background, foreground;

			/** If the specified color type is "normal" for this control, this function searches the control's parents
			 *  until it finds one with a color of the same type and returns it. Otherwise, this returns the control's
			 *  specified color. */
			ansi::color find_color(ansi::color_type) const;

		public:
			colored(ansi::color foreground, ansi::color background): background(background), foreground(foreground) {}
			colored(ansi::color foreground): colored(foreground, ansi::color::normal) {}
			colored(): colored(ansi::color::normal) {}

			ansi::color get_background() const { return background; }
			ansi::color get_foreground() const { return foreground; }
			void set_background(ansi::color background_) { background = background_; draw(); }
			void set_foreground(ansi::color foreground_) { foreground = foreground_; draw(); }
			/** Sets the foreground and background colors of the control. */
			void set_colors(ansi::color foreground_, ansi::color background_);
			/** Applies the control's colors to the terminal. */
			void apply_colors();
			/** Applies the control's colors to the terminal if they are not the currently active colors. */
			void try_colors(bool find = false);
			/** Resets the terminal's colors. */
			void uncolor();

			virtual void draw() override;
			virtual void focus() override;
	};
}

#endif
