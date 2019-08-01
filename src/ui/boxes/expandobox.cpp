#include <algorithm>

#include "ui/boxes/expandobox.h"

namespace haunted::ui::boxes {
	expandobox::expandobox(std::initializer_list<std::pair<control *, int>> pairs) {
		for (const std::pair<control *, int> &p: pairs) {
			children.push_back(p.first);
			sizes.push_back(p.second);
		}
	}

	int expandobox::fixed_sum() const {
		int sum = 0;
		for (const int size: sizes) {
			if (size > 0)
				sum += size;
		}

		return sum;
	}

	int expandobox::expanded_size(int order) const {
		if (children.empty())
			return 0;

		const int sum = fixed_sum();
		const int length = get_size();

		// If the fixed-size children fill the expandobox or even overflow it,
		// there's no room for the expanding children.
		if (length <= sum)
			return 0;
		
		const int quotient  = (length - sum) / children.size();
		const int remainder = (length - sum) % children.size();

		return quotient + (order < remainder? 1 : 0);
	}

	void expandobox::resize_child(control *child, int offset, int size) {
		if (orientation == horizontal) {
			child->resize({pos.left + offset, pos.top, size, pos.height});
		} else {
			child->resize({pos.left, pos.top + offset, pos.width, size});
		}
	}

	void expandobox::resize(const position &new_pos) {
		control::resize(new_pos);
		const int size = get_size();

		// Number of times an expanding child has been resized.
		int expanded = 0;

		// The amount of space that has been allocated to children so far.
		int offset = 0;

		for (const std::pair<control *, int> p: *this) {
			control *child = p.first;
			const int child_size = p.second;

			if (offset >= size) {
				// If there's no space left, assign the child a size of zero and place it at the far edge.
				resize_child(child, get_size(), 0);
			} else {
				const int assigned = size == -1? expanded_size(expanded++) : std::min(child_size, size - offset);
				resize_child(child, offset, assigned);
				offset += assigned;
			}
		}
	}

	int expandobox::max_children() const {
		return -1;
	}

	expandobox & expandobox::operator+=(expandobox::child_pair p) {
		children.push_back(p.first);
		sizes.push_back(p.second);
		return *this;
	}

	expandobox::iterator expandobox::begin() {
		return {children.begin(), sizes.begin()};
	}

	expandobox::iterator expandobox::end() {
		return {children.end(), sizes.end()};
	}
}
