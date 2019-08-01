#ifndef HAUNTED_UI_BOXES_PROPOBOX_H_
#define HAUNTED_UI_BOXES_PROPOBOX_H_

#include "ui/boxes/box.h"
#include "ui/boxes/dualbox.h"

namespace haunted::ui::boxes {
	/**
	 * Represents a box whose two children's lengths are determined with a ratio. 
	 * For example, if the ratio is 1.5, the first child's size will be 1.5x
	 * the second child's size. For a box size of 5, the first child's size will
	 * be 3 and the second child's will be 2.
	 * 
	 *     size_one = ratio * size_box/(1 + ratio)
	 *              = size_box - size_two
	 *              = size_box - size_box/(1 + ratio)
	 *              = size_box * (1 - 1/(1 + ratio))
	 * 
	 *     size_two = size_box/(1 + ratio)
	 */
	class propobox: public dualbox {
		protected:
			propobox(const position &, double);
			propobox(const position &pos_): propobox(pos_, 1) {}
			double ratio;

		public:
			double get_ratio() const { return ratio; }
			void set_ratio(const double);

			virtual int size_one() const override;
			virtual int size_two() const override;
			virtual int get_size() const override;
	};
}

#endif
