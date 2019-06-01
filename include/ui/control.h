#ifndef HAUNTED_UI_CONTROL_H_
#define HAUNTED_UI_CONTROL_H_

#include <vector>

#include "../core/defs.h"
#include "control.h"

namespace haunted::ui {
	/**
	 * Represents a control.
	 * This includes things like boxes, text views and text inputs.
	 */
	class control {
		protected:
			std::vector<control *> children;
			haunted::position pos;
			control(const haunted::position &pos_): pos(pos_) {}

		public:
			virtual ~control() = 0;
			virtual int max_children() const;
			virtual int  child_count() const;
			virtual void draw() = 0;
			virtual void resize(const haunted::position &);

			control * operator[](size_t);
	};
}

#endif
