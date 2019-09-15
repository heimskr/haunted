#ifndef HAUNTED_UI_BOXES_BOX_H_
#define HAUNTED_UI_BOXES_BOX_H_

#include "core/hdefs.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted::ui::boxes {
	/**
	 * Base class representing boxes, which are controls that contain at least two subcontrols
	 * and resize them as appropriate.
	 */
	class box: public virtual container, public control {
		public:
			box() = delete;
			box(container *parent_, const position &pos_ = {}): control(parent_, pos_) {}

			virtual terminal * get_terminal() override { return term; }
	};
}

#endif
