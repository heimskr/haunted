#ifndef HAUNTED_UI_BOXES_ORIENTEDBOX_H_
#define HAUNTED_UI_BOXES_ORIENTEDBOX_H_

#include "ui/boxes/box.h"

namespace haunted::ui::boxes {
	enum box_orientation {horizontal, vertical};

	class orientedbox: public box {
		protected:
			box_orientation orientation;

		public:
			using box::box;

			orientedbox(container *parent, const haunted::position &pos, box_orientation orientation):
				box(parent, pos), orientation(orientation) {}

			orientedbox(const haunted::position &pos, box_orientation orientation):
				box(pos), orientation(orientation) {}

			orientedbox(container *parent, terminal *term, box_orientation orientation):
				box(parent, term), orientation(orientation) {}

			orientedbox(container *parent, box_orientation orientation):
				box(parent), orientation(orientation) {}

			/** Returns the length of the axis of orientation.
			 *  For horizontal boxes, this is the width of the box. For vertical boxes, it's the height. */
			virtual int get_size() const;
	};
}

#endif
