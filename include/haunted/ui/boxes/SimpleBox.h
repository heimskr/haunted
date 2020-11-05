#ifndef HAUNTED_UI_BOXES_SIMPLEBOX_H_
#define HAUNTED_UI_BOXES_SIMPLEBOX_H_

#include "haunted/core/Defs.h"
#include "haunted/ui/boxes/Box.h"
#include "haunted/ui/Container.h"
#include "haunted/ui/Control.h"

namespace Haunted::UI::Boxes {
	/**
	 * Represents a simple box that can contain one child. When the box is moved or resized, the child is moved to the
	 * same position as the box but not resized.
	 */
	class SimpleBox: public virtual Box {
		protected:

		public:
			using Box::Box;

			virtual int maxChildren() const override { return 1; }
			virtual void resize(const Position &) override;
			virtual void move(int, int) override;
			virtual void draw() override;
	};
}

#endif
