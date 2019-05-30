#ifndef HAUNTED_UI_BOXES_HBOX_PROPO_H_
#define HAUNTED_UI_BOXES_HBOX_PROPO_H_

#include "ui/boxes/box_dual.h"
#include "ui/boxes/box_propo.h"
#include "ui/boxes/hbox.h"

namespace haunted::ui::boxes {
	class hbox_propo: public virtual hbox, public virtual box_propo {
		public:
			using box_dual::max_children;
	};
}

#endif
