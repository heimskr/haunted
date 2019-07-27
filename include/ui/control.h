#ifndef HAUNTED_UI_CONTROL_H_
#define HAUNTED_UI_CONTROL_H_

#include "../core/defs.h"
#include "../core/key.h"
#include "child.h"
#include "container.h"
#include "control.h"
#include "keyhandler.h"

namespace haunted::ui {
	/**
	 * Represents a control.
	 * This includes things like boxes, text views and text inputs.
	 */
	class control: public keyhandler, public child {
		protected:
			terminal *term;
			haunted::position pos;
			control(container *parent, const haunted::position &pos):
				child(parent), term(parent->get_term()), pos(pos) {}
			control(const haunted::position &pos):
				child(nullptr), term(nullptr), pos(pos) {}

		public:
			control(container *parent, terminal *term):
				child(parent), term(term) {}
			control(container *parent):
				control(parent, parent->get_term()) {}
			control(): control(nullptr, nullptr) {}

			virtual ~control() = 0;
			
			/** Renders the component on the terminal. */
			virtual void draw() = 0;
			virtual void resize(const haunted::position &);
			virtual void move(int left, int top);
			virtual void focus();

			haunted::position get_position() const;

			/** Moves the cursor on the screen to the top-left corner of the control. */
			void jump();

			/** Erases the portion of the display that this control occupies. */
			void clear_rect();

			/** Flushes the terminal's output buffer. */
			void flush();

			/** Returns true if the control is its terminal's focused control. */
			bool has_focus() const;

			/** Returns true if the control's right edge is at the right edge of the screen. */
			bool at_right() const;

			/** Returns true if the control's left edge is at the left edge of the screen. */
			bool at_left() const;

			/** Sets the terminal's scrollable region with DECSLRM and DECSTBM to fit the control. */
			void set_margins();

			/** Resets the scrollable region. */
			void reset_margins();

			friend class container;
	};
}

#endif
