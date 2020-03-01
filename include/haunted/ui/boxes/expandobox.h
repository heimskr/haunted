#ifndef HAUNTED_UI_BOXES_EXPANDOBOX_H_
#define HAUNTED_UI_BOXES_EXPANDOBOX_H_

#include <deque>
#include <utility>

#include "haunted/ui/colored.h"
#include "haunted/ui/boxes/orientedbox.h"

namespace Haunted::UI::Boxes {
	/**
	 * Represents a box that contains some number of children with fixed sizes
	 * and one or more children that expand to fill the remaining space.
	 * The remaining space is distributed equally among those children.
	 * TODO: possibly create an unholy monstrosity that supports ratios/weights for the expanding children?
	 */
	class expandobox: public orientedbox, public colored {
		private:
			/** Returns the sum of all the fixed sizes. */
			int fixed_sum() const;

			/** Returns the size for the nth expanding child. (If the space available for all the expanding children
			 *  isn't divisible by the number of expanding children and R is the remainder, then the first R expanding
			 *  children will have the ceiling of the quotient and the rest will have the floor.) */
			int expanded_size(int order) const;

			/** Resizes a child with a given size. */
			void resize_child(control *child, int offset, int size);

			template <typename L, typename R>
			class pair_iterator: public std::iterator<std::forward_iterator_tag, std::pair<L, R>> {
				private:
					std::deque<control *>::iterator child_iterator;
					std::deque<int>::iterator size_iterator;

				public:
					pair_iterator(std::deque<control *>::iterator c, std::deque<int>::iterator s):
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
			typedef std::pair<control *, int> child_pair;
			typedef pair_iterator<control *, int> iterator;

			expandobox(Container *, const position &, const box_orientation, std::initializer_list<child_pair>);
			expandobox(Container *parent, const position &pos, const box_orientation orientation):
				expandobox(parent, pos, orientation, {}) {}
			expandobox(Container *parent, const box_orientation orientation = box_orientation::horizontal,
				std::initializer_list<child_pair> child_pairs = {}): expandobox(parent, {}, orientation, child_pairs) {}

			using control::resize;
			void resize(const position &) override;
			void draw() override;
			int max_children() const override { return -1; }
			bool request_resize(control *, size_t, size_t) override;

			virtual Terminal * get_terminal() override { return term; }
			virtual Container * get_parent() const override { return parent; }

			expandobox & operator+=(child_pair);

			iterator begin();
			iterator end();
	};
}

#endif
