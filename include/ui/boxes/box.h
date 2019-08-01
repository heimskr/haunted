#ifndef HAUNTED_UI_BOXES_BOX_H_
#define HAUNTED_UI_BOXES_BOX_H_

#include "core/defs.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted::ui::boxes {
	/**
	 * Base class representing boxes, which are controls that contain at least two subcontrols
	 * and resize them as appropriate.
	 */
	class box: public container, public control {
		protected:
			box(const position &pos): control(pos) {}
			using control::control;

		public:
			haunted::side side;

			/** Returns the maximum number of children the box can contain (-1 for unlimited). */
			virtual int max_children() const = 0;
	};
}

#endif
