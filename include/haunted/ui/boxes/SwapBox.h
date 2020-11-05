#ifndef HAUNTED_UI_BOXES_SWAPBOX_H_
#define HAUNTED_UI_BOXES_SWAPBOX_H_

#include <list>

#include "haunted/core/Defs.h"
#include "haunted/ui/boxes/Box.h"
#include "haunted/ui/Container.h"
#include "haunted/ui/Control.h"

namespace Haunted::UI::Boxes {
	/**
	 * Represents a box that contains some number of controls. At most one is active at any given time; the others are
	 * kept in memory but aren't drawn.
	 */
	class SwapBox: public virtual Box {
		protected:
			Control *active = nullptr;

		public:
			SwapBox(const SwapBox &) = delete;
			SwapBox(Container *, const Position &, std::initializer_list<Control *> = {});

			void setActive(Control *);
			Control * getActive() { return active; }

			/** Returns the active control if the given coordinates are within the SwapBox's area. */
			virtual Control * childAtOffset(int x, int y) const override;

			virtual void resize(const Position &) override;
			virtual void draw() override;
			bool onKey(const Key &) override;
	};
}

#endif
