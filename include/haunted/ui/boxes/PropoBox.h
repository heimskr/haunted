#ifndef HAUNTED_UI_BOXES_PROPOBOX_H_
#define HAUNTED_UI_BOXES_PROPOBOX_H_

#include "haunted/ui/boxes/Box.h"
#include "haunted/ui/boxes/DualBox.h"
#include "haunted/ui/Colored.h"

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
	class PropoBox: public DualBox, public Colored {
		protected:
			PropoBox(Container *, const Position &, double, BoxOrientation);
			double ratio;

		public:
			PropoBox(Container *, double, BoxOrientation, Control * = nullptr, Control * = nullptr,
				const Position & = {});
			double getRatio() const { return ratio; }
			void setRatio(const double);

			using Control::resize;
			void resize(const Position &) override;

			virtual void draw() override;
			virtual int maxChildren() const override { return 2; }
			virtual int sizeOne() const override;
			virtual int sizeTwo() const override;
			virtual int getSize() const override;

			virtual Terminal * getTerminal() override { return terminal; }
			virtual Container * getParent() const override { return parent; }
	};
}

#endif
