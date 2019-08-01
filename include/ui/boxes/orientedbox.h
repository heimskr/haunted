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

			/** Returns the length of the axis of orientation.
			 *  For horizontal boxes, this is the width of the box. For vertical boxes, it's the height. */
			virtual int get_size() const;
	};
}

#endif
