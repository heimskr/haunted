#ifndef HAUNTED_UI_BOXES_EXPANDOBOX_H_
#define HAUNTED_UI_BOXES_EXPANDOBOX_H_

#include <deque>
#include <utility>

#include "haunted/ui/Colored.h"
#include "haunted/ui/boxes/OrientedBox.h"

namespace Haunted::UI::Boxes {
	/**
	 * Represents a box that contains some number of children with fixed sizes
	 * and one or more children that expand to fill the remaining space.
	 * The remaining space is distributed equally among those children.
	 * TODO: possibly create an unholy monstrosity that supports ratios/weights for the expanding children?
	 */
	class ExpandoBox: public OrientedBox, public Colored {
		private:
			/** Returns the sum of all the fixed sizes. */
			int fixedSum() const;

			/** Returns the size for the nth expanding child. (If the space available for all the expanding children
			 *  isn't divisible by the number of expanding children and R is the remainder, then the first R expanding
			 *  children will have the ceiling of the quotient and the rest will have the floor.) */
			int expandedSize(int order) const;

			/** Resizes a child with a given size. */
			void resizeChild(Control *child, int offset, int size);

			template <typename L, typename R>
			class pair_iterator: public std::iterator<std::forward_iterator_tag, std::pair<L, R>> {
				private:
					std::deque<Control *>::iterator child_iterator;
					std::deque<int>::iterator size_iterator;

				public:
					pair_iterator(std::deque<Control *>::iterator c, std::deque<int>::iterator s):
						child_iterator(c), size_iterator(s) {}

					std::pair<L &, R &> operator*() const;

					pair_iterator & operator++();
					pair_iterator operator++(int);

					bool operator==(pair_iterator);
					bool operator!=(pair_iterator);

					operator pair_iterator<L const, R const>() const;
			};

		protected:
			/** Contains the sizes of all the children. Children that expand have a size of -1. */
			std::deque<int> sizes;

		public:
			using ChildPair = std::pair<Control *, int>;
			using iterator  = pair_iterator<Control *, int>;

			ExpandoBox(Container *, const Position &, const BoxOrientation, std::initializer_list<ChildPair>);
			ExpandoBox(Container *parent, const Position &pos, const BoxOrientation orientation):
				ExpandoBox(parent, pos, orientation, {}) {}
			ExpandoBox(Container *parent, const BoxOrientation orientation = BoxOrientation::Horizontal,
				std::initializer_list<ChildPair> child_pairs = {}): ExpandoBox(parent, {}, orientation, child_pairs) {}

			using Control::resize;
			void resize(const Position &) override;
			void draw() override;
			int maxChildren() const override { return -1; }
			bool requestResize(Control *, size_t, size_t) override;

			virtual Terminal * getTerminal() override { return terminal; }
			virtual Container * getParent() const override { return parent; }

			ExpandoBox & operator+=(const ChildPair &);

			iterator begin();
			iterator end();
	};
}

#endif
