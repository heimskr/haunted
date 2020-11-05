#ifndef HAUNTED_UI_BOXES_BOX_H_
#define HAUNTED_UI_BOXES_BOX_H_

#include "haunted/core/Defs.h"
#include "haunted/ui/Container.h"
#include "haunted/ui/Control.h"

namespace Haunted::UI::Boxes {
	/**
	 * Base class representing boxes, which are controls that contain at least two subcontrols
	 * and resize them as appropriate.
	 */
	class Box: public virtual Container, public Control {
		public:
			Box() = delete;
			Box(Container *parent_, const Position &pos_ = {}): Control(parent_, pos_) {}

			virtual Terminal * getTerminal() override { return terminal; }

			virtual Position getPosition() const override { return Control::getPosition(); }
	};
}

#endif
