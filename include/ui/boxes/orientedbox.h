#ifndef HAUNTED_UI_BOXES_ORIENTEDBOX_H_
#define HAUNTED_UI_BOXES_ORIENTEDBOX_H_

#include "ui/boxes/box.h"

namespace haunted::ui::boxes {
	enum class box_orientation {horizontal, vertical};

	class orientedbox: public box {
		protected:
			box_orientation orientation;

		public:
			orientedbox() = delete;
			orientedbox(orientedbox &&) = delete;
			orientedbox(const orientedbox &) = delete;

			orientedbox(container *parent_, const haunted::position &pos_, box_orientation orientation_):
			box(parent_, pos_), orientation(orientation_) {
				DBG("Regards from orientedbox::orientedbox(container *, const position &, box_orientation). parent = " << parent << ", parent_" << parent_);
			}

			orientedbox(container *parent_, box_orientation orientation_): orientedbox(parent_, {}, orientation_) {}

			/** Returns the length of the axis of orientation.
			 *  For horizontal boxes, this is the width of the box. For vertical boxes, it's the height. */
			virtual int get_size() const;

			/** Changes the box's orientation and makes any necessary adjustments. */
			virtual void set_orientation(box_orientation);
	};
}

#endif
