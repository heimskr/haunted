#ifndef HAUNTED_UI_BOXES_BOX_DUAL_H_
#define HAUNTED_UI_BOXES_BOX_DUAL_H_

#include "ui/boxes/box.h"

namespace haunted::ui::boxes {
	class box_dual: public virtual box {
		protected:
			using box::box;
			box_orientation orientation;

		public:
			virtual void resize(const position &) override;
			virtual int max_children() const override;
			virtual int size_one() const = 0;
			virtual int size_two() const = 0;
			virtual int get_size() const = 0;
	};
}

#endif
