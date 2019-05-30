#ifndef HAUNTED_BOXES_BOX_H_
#define HAUNTED_BOXES_BOX_H_

#include "haunted/defs.h"
#include "haunted/control.h"

namespace haunted::boxes {
	/**
	 * Base class representing boxes, which are controls that contain
	 * two subcontrols and provide resize them as appropriate.
	 */
	class box: public virtual control {
		protected:
			box(const position &pos_): control(pos_) {}
			using control::control;

		public:
			haunted::side side;

			virtual int max_children() const = 0;
	};
}

#endif
