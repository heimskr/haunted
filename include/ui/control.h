#ifndef HAUNTED_UI_CONTROL_H_
#define HAUNTED_UI_CONTROL_H_

#include "../core/defs.h"
#include "container.h"
#include "control.h"

namespace haunted::ui {
	class terminal: public virtual container;

	/**
	 * Represents a control.
	 * This includes things like boxes, text views and text inputs.
	 */
	class control {
		protected:
			container *parent = nullptr;
			terminal *term;
			haunted::position pos;
			control(const haunted::position &pos_): pos(pos_) {}

		public:
			control(container *parent, terminal *term):
				parent(parent), term(term) {}
			control(container *parent):
				control(parent, parent->get_term()) {}
			control(): control(nullptr, nullptr) {}

			virtual ~control() = 0;
			
			virtual void draw() = 0;
			virtual void resize(const haunted::position &);
			void jump();

			friend class container;
	};
}

#endif
