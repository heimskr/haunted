#ifndef HAUNTED_UI_CONTROL_H_
#define HAUNTED_UI_CONTROL_H_

#include "../core/defs.h"
#include "container.h"
#include "control.h"

namespace haunted::ui {
	/**
	 * Represents a control.
	 * This includes things like boxes, text views and text inputs.
	 */
	class control {
		protected:
			container *parent = nullptr;
			haunted::position pos;
			control(const haunted::position &pos_): pos(pos_) {}

		public:
			control(container *parent): parent(parent) {}
			control(): control(nullptr) {}

			virtual ~control() = 0;
			virtual void resize(const haunted::position &);
			virtual void draw() = 0;

			friend class container;
	};
}

#endif
