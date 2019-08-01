#ifndef HAUNTED_UI_BOXES_BOX_FIXED_H_
#define HAUNTED_UI_BOXES_BOX_FIXED_H_

#include "ui/boxes/box.h"

namespace haunted::ui::boxes {
	/**
	 * Represents a box that contains some number of children with fixed sizes
	 * and one or more children that expand to fill the remaining space.
	 */
	class expandobox: public virtual box {
		protected:
			using box::box;
			box_orientation orientation;

		public:
			virtual void resize(const position &) override;
			virtual int max_children() const override;
	};
}

#endif
