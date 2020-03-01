#include <algorithm>

#include "haunted/core/terminal.h"
#include "haunted/ui/boxes/expandobox.h"

namespace Haunted::UI::Boxes {
	template <>
	std::pair<control *&, int &> expandobox::iterator::operator*() const {
		return {*child_iterator, *size_iterator};
	}

	template <>
	expandobox::iterator & expandobox::iterator::operator++() {
		++child_iterator;
		++size_iterator;
		return *this;
	}

	template <>
	expandobox::iterator expandobox::iterator::operator++(int) {
		std::deque<Control *>::iterator new_child_iterator(child_iterator);
		std::deque<int>::iterator new_size_iterator(size_iterator);
		return {++new_child_iterator, ++new_size_iterator};
	}

	template <>
	bool expandobox::iterator::operator==(expandobox::iterator other) {
		return child_iterator == other.child_iterator && size_iterator == other.size_iterator;
	}

	template <>
	bool expandobox::iterator::operator!=(expandobox::iterator other) {
		return child_iterator != other.child_iterator || size_iterator != other.size_iterator;
	}

	expandobox::expandobox(container *parent_, const position &pos, const box_orientation orientation,
	std::initializer_list<child_pair> pairs): orientedbox(parent_, pos, orientation) {
		if (parent_)
			parent_->add_child(this);

		for (const child_pair &p: pairs) {
			p.first->set_parent(this);
			p.first->set_terminal(term);
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

		int expanding_count = 0;
		for (const int size: sizes) {
			if (size == -1)
				++expanding_count;
		}
		
		if (expanding_count == 0)
			return 0;
		
		const int quotient  = (length - sum) / expanding_count;
		const int remainder = (length - sum) % expanding_count;

		return quotient + (order < remainder? 1 : 0);
	}

	void expandobox::resize_child(control *child, int offset, int size) {
		if (orientation == box_orientation::horizontal) {
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

		for (const child_pair p: *this) {
			control *child = p.first;
			const int child_size = p.second;

			if (offset >= size) {
				// If there's no space left, assign the child a size of zero and place it at the far edge.
				DBG("Resizing " << child->get_id() << " with offset " << get_size() << " and size " << 0 << " (no space left)");
				resize_child(child, get_size(), 0);
			} else {
				const int assigned = child_size == -1? expanded_size(expanded++) : std::min(child_size, size - offset);
				resize_child(child, offset, assigned);
				offset += assigned;
			}
		}

		redraw();
	}

	void expandobox::draw() {
		if (!can_draw())
			return;

		colored::draw();

		auto lock = term->lock_render();
		for (control *child: children)
			child->draw();
	}

	bool expandobox::request_resize(control *child, size_t width, size_t height) {
		// Don't try to resize anything that isn't a direct descendant.
		for (auto iter = children.begin(), end = children.end(); iter != end; ++iter) {
			if (*iter == child) {
				size_t pos = iter - children.begin();
				sizes[pos] = orientation == box_orientation::vertical? height : width;
				resize();
				return true;
			}
		}

		return false;
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
