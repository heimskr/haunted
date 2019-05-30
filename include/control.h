#ifndef HAUNTED_CONTROL_H_
#define HAUNTED_CONTROL_H_

#include <vector>

#include "haunted/defs.h"

namespace haunted {
	/**
	 * Represents a control.
	 * This includes things like boxes, text views and text inputs.
	 */
	class control {
		protected:
			std::vector<control *> children;
			position pos;
			control(const position &pos_): pos(pos_) {}

		public:
			virtual ~control() = 0;
			virtual int max_children() const;
			virtual int  child_count() const;
			virtual void resize(const position &new_pos) = 0;

			control * operator[](size_t);
	};
}

#endif
