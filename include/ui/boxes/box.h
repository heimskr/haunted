#ifndef HAUNTED_UI_BOXES_BOX_H_
#define HAUNTED_UI_BOXES_BOX_H_

#include "core/defs.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted::ui::boxes {
	/**
	 * Base class representing boxes, which are controls that contain at least two subcontrols
	 * and resize them as appropriate.
	 */
	class box: public virtual container, public virtual control {
		protected:
			// box(const position &pos): control(pos) { DBG("box::box(pos): pos " << pos); }
			// box(container *parent): control(parent) { DBG("box::box(parent): parent " << parent); }
			// box(container *parent, terminal *term): control(parent, term) { DBG("box::box(parent, term): parent " << parent << ", term " << term); }
			// box(container *parent, const position &pos): control(parent, pos) { DBG("box::box(parent, pos): parent " << parent << ", pos " << pos); }
			// box(): control(nullptr, nullptr) { DBG("box::box()"); }
			box(const position &pos): control(pos) { DBGFN(); }
			box(container *parent): control(parent) { DBGFN(); }
			box(container *parent, terminal *term): control(parent, term) { DBGFN(); }
			box(container *parent, const position &pos): control(parent, pos) { DBGFN(); }
			box(): control(nullptr, nullptr) { DBGFN(); }

		public:
			haunted::side side;

			/** Returns the maximum number of children the box can contain (-1 for unlimited). */
			virtual int max_children() const = 0;
	};
}

#endif
