#ifndef HAUNTED_UI_BOXES_ORIENTEDBOX_H_
#define HAUNTED_UI_BOXES_ORIENTEDBOX_H_

#include "haunted/ui/boxes/Box.h"

namespace Haunted::UI::Boxes {
	enum class BoxOrientation {Horizontal, Vertical};

	class OrientedBox: public Box {
		protected:
			BoxOrientation orientation;

		public:
			OrientedBox() = delete;
			OrientedBox(OrientedBox &&) = delete;
			OrientedBox(const OrientedBox &) = delete;

			OrientedBox(Container *parent_, const Haunted::Position &pos_, BoxOrientation orientation_):
				Box(parent_, pos_), orientation(orientation_) {}

			OrientedBox(Container *parent_, BoxOrientation orientation_):
				OrientedBox(parent_, {}, orientation_) {}

			/** Returns the length of the axis of orientation.
			 *  For horizontal boxes, this is the width of the box. For vertical boxes, it's the height. */
			virtual int getSize() const;

			/** Changes the box's orientation and makes any necessary adjustments. */
			virtual void setOrientation(BoxOrientation);
	};
}

#endif
