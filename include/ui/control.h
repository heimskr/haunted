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
			control(const haunted::position &pos_):
				child(nullptr), pos(pos_) {}

		public:
			control(container *parent, terminal *term):
				child(parent), term(term) {}
			control(container *parent):
				control(parent, parent->get_term()) {}
			control(): control(nullptr, nullptr) {}

			virtual ~control() = 0;
			
			virtual void draw() = 0;
			virtual void resize(const haunted::position &);
			void jump();

			virtual void focus();

			friend class container;
	};
}

#endif
