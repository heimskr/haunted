#ifndef HAUNTED_UI_BOXES_BOX_H_
#define HAUNTED_UI_BOXES_BOX_H_

#include "haunted/core/defs.h"
#include "haunted/ui/container.h"
#include "haunted/ui/control.h"

namespace Haunted::UI::Boxes {
	/**
	 * Base class representing boxes, which are controls that contain at least two subcontrols
	 * and resize them as appropriate.
	 */
	class Box: public virtual Container, public Control {
		public:
			Box() = delete;
			Box(Container *parent_, const position &pos_ = {}): Control(parent_, pos_) {}

			virtual Terminal * get_terminal() override { return term; }

			virtual position get_position() const override { return Control::get_position(); }
	};
}

#endif
