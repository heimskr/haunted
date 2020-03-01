#ifndef HAUNTED_UI_BOXES_ORIENTEDBOX_H_
#define HAUNTED_UI_BOXES_ORIENTEDBOX_H_

#include "haunted/ui/boxes/box.h"

namespace Haunted::UI::Boxes {
	enum class box_orientation {horizontal, vertical};

	class orientedbox: public Box {
		protected:
			box_orientation orientation;

		public:
			orientedbox() = delete;
			orientedbox(orientedbox &&) = delete;
			orientedbox(const orientedbox &) = delete;

			orientedbox(Container *parent_, const Haunted::position &pos_, box_orientation orientation_):
			Box(parent_, pos_), orientation(orientation_) {}

			orientedbox(Container *parent_, box_orientation orientation_): orientedbox(parent_, {}, orientation_) {}

			/** Returns the length of the axis of orientation.
			 *  For horizontal boxes, this is the width of the box. For vertical boxes, it's the height. */
			virtual int get_size() const;

			/** Changes the box's orientation and makes any necessary adjustments. */
			virtual void set_orientation(box_orientation);
	};
}

#endif
