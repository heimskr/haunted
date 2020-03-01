#ifndef HAUNTED_UI_BOXES_SIMPLEBOX_H_
#define HAUNTED_UI_BOXES_SIMPLEBOX_H_

#include "haunted/core/defs.h"
#include "haunted/ui/boxes/box.h"
#include "haunted/ui/container.h"
#include "haunted/ui/control.h"

namespace haunted::ui::boxes {
	/**
	 * Represents a simple box that can contain one child. When the box is moved or resized, the child is moved to the
	 * same position as the box but not resized.
	 */
	class simplebox: public virtual box {
		protected:

		public:
			using box::box;

			virtual int max_children() const override { return 1; }
			virtual void resize(const position &) override;
			virtual void move(int, int) override;
			virtual void draw() override;
	};
}

#endif
