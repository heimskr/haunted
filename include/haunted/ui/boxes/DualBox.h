#ifndef HAUNTED_UI_BOXES_DUALBOX_H_
#define HAUNTED_UI_BOXES_DUALBOX_H_

#include "haunted/ui/boxes/OrientedBox.h"

namespace Haunted::UI::Boxes {
	/**
	 * Base class for boxes that contain exactly two children.
	 */
	class DualBox: public OrientedBox {
		public:
			using OrientedBox::OrientedBox;

			DualBox() = delete;
			DualBox(DualBox &&) = delete;
			DualBox(const DualBox &) = delete;

			virtual void resize(const Position &) override;
			virtual int maxChildren() const override { return 2; }
			virtual int sizeOne() const = 0;
			virtual int sizeTwo() const = 0;
	};
}

#endif
