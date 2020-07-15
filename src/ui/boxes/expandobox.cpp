#include <algorithm>

#include "haunted/core/terminal.h"
#include "haunted/ui/boxes/expandobox.h"

namespace haunted::ui::boxes {
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
		std::deque<control *>::iterator new_child_iterator(child_iterator);
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

		for (size_t i = 0, max = children.size(); i < max; ++i) {
		// for (const child_pair p: *this) {
			// control *child = p.first;
			control *child = children[i];
			const int child_size = sizes.size() <= i? -1 : sizes[i];
			// const int child_size = p.second;
			DBG("child: " << child << ", size: " << child_size);

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
				DBG("Found child " << child->get_id());
				size_t index = iter - children.begin();
				position pos = child->get_position();
				if (orientation == box_orientation::vertical) {
					sizes[index] = height;
					pos.height = height;
				} else {
					sizes[index] = width;
					pos.width = width;
				}
				child->resize(pos);
				DBG("new child position: " << pos);
				resize();
				return true;
			}
		}

		return false;
	}

	bool expandobox::add_child(control *ctrl) {
		const bool success = container::add_child(ctrl);
		if (success) {
			if (sizes.size() < children.size()) {
				sizes.push_back(orientation == box_orientation::vertical?
					ctrl->get_position().height : ctrl->get_position().width);
			} else {
				sizes[children.size() - 1] = orientation == box_orientation::vertical?
					ctrl->get_position().height : ctrl->get_position().width;
			}
		}

		return success;
	}

	bool expandobox::remove_child(child *ctrl) {
		for (size_t i = 0, max = children.size(); i < max; ++i) {
			if (children[i] == ctrl) {
				children.erase(std::next(children.begin(), i));
				sizes.erase(std::next(sizes.begin(), i));
				return true;
			}
		}

		return false;
	}

	bool expandobox::add_child(control *ctrl, int size) {
		const bool success = container::add_child(ctrl);
		if (success) {
			if (sizes.size() < children.size())
				sizes.push_back(size);
			else
				sizes[children.size() - 1] = size;
		}

		return success;
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
