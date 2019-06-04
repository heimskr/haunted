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
				child(parent), pos(pos) {}
			control(const haunted::position &pos):
				control(nullptr, pos) {}

		public:
			control(container *parent, terminal *term):
				child(parent), term(term) {}
			control(container *parent):
				control(parent, parent->get_term()) {}
			control(): control(nullptr, nullptr) {}

			virtual ~control() = 0;
			
			virtual void draw() = 0;
			virtual void resize(const haunted::position &);

			/** Moves the cursor on the screen to the top-left corner of the control. */
			void jump();

			/** Erases the portion of the display that this control occupies. */
			void clear_rect();

			virtual void focus();

			friend class container;
	};
}

#endif
