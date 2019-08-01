#ifndef HAUNTED_UI_BOXES_DUALBOX_H_
#define HAUNTED_UI_BOXES_DUALBOX_H_

#include "ui/boxes/box.h"

namespace haunted::ui::boxes {
	/**
	 * Base class for boxes that contain exactly two children.
	 */
	class dualbox: public virtual box {
		protected:
			using box::box;
			box_orientation orientation;

		public:
			virtual void resize(const position &) override;
			virtual int max_children() const override;
			virtual int size_one() const = 0;
			virtual int size_two() const = 0;

			/** Returns the length of the axis that spans both boxes.
			 *  For horizontal boxes, this is the width of the box. For vertical boxes, it's the height. */
			virtual int get_size() const = 0;
	};
}

#endif
