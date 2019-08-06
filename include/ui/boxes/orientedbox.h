#ifndef HAUNTED_UI_BOXES_ORIENTEDBOX_H_
#define HAUNTED_UI_BOXES_ORIENTEDBOX_H_

#include "ui/boxes/box.h"

namespace haunted::ui::boxes {
	enum class box_orientation {horizontal, vertical};

	class orientedbox: public virtual box {
		protected:
			box_orientation orientation;

		public:
			using box::box;
			//*
			// orientedbox(const haunted::position &pos): box(pos) {
			// 	// DBG("orientedbox::orientedbox(pos)");
			// 	DBGFN();
			// }
			
			orientedbox(container *parent, const haunted::position &pos, box_orientation orientation):
				box(parent, pos), orientation(orientation) {
					// DBG("oriented[" << this << "]: parent " << parent << ", pos " << pos << ", orientation " << (orientation == haunted::ui::boxes::box_orientation::horizontal? "h" : "v"));
					DBGFN();
				}

			orientedbox(const haunted::position &pos, box_orientation orientation):
				box(pos), orientation(orientation) {
					// DBG("oriented[" << this << "]: pos " << pos << ", orientation " << (orientation == haunted::ui::boxes::box_orientation::horizontal? "h" : "v"));
					DBGFN();
				}

			orientedbox(container *parent, terminal *term, box_orientation orientation):
				box(parent, term), orientation(orientation) {
					// DBG("oriented[" << this << "]: parent " << parent << ", term " << term << ", orientation " << (orientation == haunted::ui::boxes::box_orientation::horizontal? "h" : "v"));
					DBGFN();
				}

			orientedbox(container *parent, box_orientation orientation):
				box(parent), orientation(orientation) {
					// DBG("oriented[" << this << "]: parent " << parent << ", orientation " << (orientation == haunted::ui::boxes::box_orientation::horizontal? "h" : "v"));
					DBGFN();
				}
			//*/

			/** Returns the length of the axis of orientation.
			 *  For horizontal boxes, this is the width of the box. For vertical boxes, it's the height. */
			virtual int get_size() const;

			/** Changes the box's orientation and makes any necessary adjustments. */
			virtual void set_orientation(box_orientation);
	};
}

#endif
