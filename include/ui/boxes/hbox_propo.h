#ifndef HAUNTED_BOXES_HBOX_PROPO_H_
#define HAUNTED_BOXES_HBOX_PROPO_H_

#include "haunted/boxes/box_dual.h"
#include "haunted/boxes/box_propo.h"
#include "haunted/boxes/hbox.h"

namespace haunted::boxes {
	class hbox_propo: public virtual hbox, public virtual box_propo {
		public:
			using box_dual::max_children;
	};
}

#endif
