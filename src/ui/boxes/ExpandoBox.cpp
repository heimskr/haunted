#include <algorithm>

#include "haunted/core/Terminal.h"
#include "haunted/ui/boxes/ExpandoBox.h"

namespace Haunted::UI::Boxes {
	template <>
	std::pair<Control *&, int &> ExpandoBox::iterator::operator*() const {
		return {*child_iterator, *size_iterator};
	}

	template <>
	ExpandoBox::iterator & ExpandoBox::iterator::operator++() {
		++child_iterator;
		++size_iterator;
		return *this;
	}

	template <>
	ExpandoBox::iterator ExpandoBox::iterator::operator++(int) {
		std::deque<Control *>::iterator new_child_iterator(child_iterator);
		std::deque<int>::iterator new_size_iterator(size_iterator);
		return {++new_child_iterator, ++new_size_iterator};
	}

	template <>
	bool ExpandoBox::iterator::operator==(ExpandoBox::iterator other) {
		return child_iterator == other.child_iterator && size_iterator == other.size_iterator;
	}

	template <>
	bool ExpandoBox::iterator::operator!=(ExpandoBox::iterator other) {
		return child_iterator != other.child_iterator || size_iterator != other.size_iterator;
	}

	ExpandoBox::ExpandoBox(Container *parent_, const Position &pos, const BoxOrientation orientation,
	std::initializer_list<ChildPair> pairs): OrientedBox(parent_, pos, orientation) {
		if (parent_)
			parent_->addChild(this);

		for (const ChildPair &p: pairs) {
			p.first->setParent(this);
			p.first->setTerminal(terminal);
			children.push_back(p.first);
			sizes.push_back(p.second);
		}
	}


	int ExpandoBox::fixedSum() const {
		int sum = 0;
		for (const int size: sizes) {
			if (size > 0)
				sum += size;
		}

		return sum;
	}

	int ExpandoBox::expandedSize(int order) const {
		if (children.empty())
			return 0;

		const int sum = fixedSum();
		const int length = getSize();

		// If the fixed-size children fill the ExpandoBox or even overflow it,
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

	void ExpandoBox::resizeChild(Control *child, int offset, int size) {
		if (orientation == BoxOrientation::Horizontal)
			child->resize({position.left + offset, position.top, size, position.height});
		else
			child->resize({position.left, position.top + offset, position.width, size});
	}

	void ExpandoBox::resize(const Position &new_pos) {
		Control::resize(new_pos);
		const int size = getSize();

		// Number of times an expanding child has been resized.
		int expanded = 0;

		// The amount of space that has been allocated to children so far.
		int offset = 0;

		for (size_t i = 0, max = children.size(); i < max; ++i) {
		// for (const child_pair p: *this) {
			// control *child = p.first;
			Control *child = children[i];
			const int child_size = sizes.size() <= i? -1 : sizes[i];
			// const int child_size = p.second;
			// DBG("child: " << child << ", size: " << child_size);

			if (offset >= size) {
				// If there's no space left, assign the child a size of zero and place it at the far edge.
				DBG("Resizing " << child->getID() << " with offset " << getSize() << " and size " << 0 << " (no space left)");
				resizeChild(child, getSize(), 0);
			} else {
				const int assigned = child_size == -1? expandedSize(expanded++) : std::min(child_size, size - offset);
				resizeChild(child, offset, assigned);
				offset += assigned;
			}
		}

		redraw();
	}

	void ExpandoBox::draw() {
		if (!canDraw())
			return;

		Colored::draw();

		auto lock = terminal->lockRender();
		for (Control *child: children)
			child->draw();
	}

	bool ExpandoBox::requestResize(Control *child, size_t width, size_t height) {
		// Don't try to resize anything that isn't a direct descendant.
		for (auto iter = children.begin(), end = children.end(); iter != end; ++iter)
			if (*iter == child) {
				DBG("Found child " << child->getID());
				size_t index = iter - children.begin();
				Position pos = child->getPosition();
				if (orientation == BoxOrientation::Vertical) {
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

		return false;
	}

	bool ExpandoBox::addChild(Control *control) {
		const bool success = Container::addChild(control);
		if (success) {
			if (sizes.size() < children.size()) {
				sizes.push_back(orientation == BoxOrientation::Vertical?
					control->getPosition().height : control->getPosition().width);
			} else {
				sizes[children.size() - 1] = orientation == BoxOrientation::Vertical?
					control->getPosition().height : control->getPosition().width;
			}
		}

		return success;
	}

	bool ExpandoBox::removeChild(Child *control) {
		for (size_t i = 0, max = children.size(); i < max; ++i) {
			if (children[i] == control) {
				children.erase(std::next(children.begin(), i));
				sizes.erase(std::next(sizes.begin(), i));
				return true;
			}
		}

		return false;
	}

	bool ExpandoBox::addChild(Control *control, int size) {
		const bool success = Container::addChild(control);
		if (success) {
			if (sizes.size() < children.size())
				sizes.push_back(size);
			else
				sizes[children.size() - 1] = size;
		}

		return success;
	}

	ExpandoBox & ExpandoBox::operator+=(const ExpandoBox::ChildPair &p) {
		children.push_back(p.first);
		sizes.push_back(p.second);
		return *this;
	}

	ExpandoBox::iterator ExpandoBox::begin() {
		return {children.begin(), sizes.begin()};
	}

	ExpandoBox::iterator ExpandoBox::end() {
		return {children.end(), sizes.end()};
	}
}
