#ifndef HAUNTED_BOXES_BOX_DUAL_H_
#define HAUNTED_BOXES_BOX_DUAL_H_

#include "haunted/boxes/box.h"

namespace haunted::boxes {
	class box_dual: public virtual box {
		protected:
			using box::box;

		public:
			virtual int max_children() const override;
			virtual int size_one() const = 0;
			virtual int size_two() const = 0;
			virtual int get_size() const = 0;
	};
}

#endif
