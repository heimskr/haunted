#ifndef HAUNTED_UI_BOXES_PROPOBOX_H_
#define HAUNTED_UI_BOXES_PROPOBOX_H_

#include "haunted/ui/boxes/box.h"
#include "haunted/ui/boxes/dualbox.h"
#include "haunted/ui/colored.h"

namespace Haunted::UI::Boxes {
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
	class propobox: public dualbox, public Haunted::UI::colored {
		protected:
			propobox(Container *, const position &, double, box_orientation);
			double ratio;

		public:
			propobox(Container *, double, box_orientation, Control * = nullptr, Control * = nullptr,
				const position & = {});
			double get_ratio() const { return ratio; }
			void set_ratio(const double);

			using Control::resize;
			void resize(const position &) override;

			virtual void draw() override;
			virtual int max_children() const override { return 2; }
			virtual int size_one() const override;
			virtual int size_two() const override;
			virtual int get_size() const override;

			virtual Terminal * get_terminal() override { return term; }
			virtual Container * get_parent() const override { return parent; }
	};
}

#endif
