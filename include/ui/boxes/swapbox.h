#ifndef HAUNTED_UI_BOXES_SWAPBOX_H_
#define HAUNTED_UI_BOXES_SWAPBOX_H_

#include <list>

#include "core/defs.h"
#include "ui/boxes/box.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted::ui::boxes {
	/**
	 * Represents a box that contains some number of controls. At most one is active at any given time; the others are
	 * kept in memory but aren't drawn.
	 */
	class swapbox: public virtual box {
		protected:
			control *active = nullptr;

		public:
			swapbox(const swapbox &) = delete;
			swapbox(container *, const position &, std::initializer_list<control *> = {});

			void set_active(control *);
			control * get_active() { return active; }

			virtual void resize(const position &) override;
			virtual void draw() override;
	};
}

#endif
