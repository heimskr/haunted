#ifndef HAUNTED_UI_BOXES_BOX_H_
#define HAUNTED_UI_BOXES_BOX_H_

#include "core/defs.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted::ui::boxes {
	/**
	 * Base class representing boxes, which are controls that contain two subcontrols and provide resize them
	 * as appropriate. This base class can contain one child. When the box is moved or resized, the child is moved to
	 * the same position as the box but not resized.
	 */
	class box: public virtual container, public virtual control {
		protected:
			box(const position &pos_): control(pos_) {}
			using control::control;

		public:
			haunted::side side;

			virtual int max_children() const override {
				// A regular box can hold one child.
				return 1;
			}

			virtual void resize(const position &) override;
			virtual void move(int, int) override;
	};
}

#endif
