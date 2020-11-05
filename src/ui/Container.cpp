#include "haunted/ui/Container.h"
#include "haunted/ui/Control.h"
#include "haunted/core/Util.h"
#include "haunted/core/Terminal.h"

namespace Haunted::UI {
	Container::~Container() {
		for (Control *child: children)
			delete child;
	}

	Control * Container::operator[](size_t index) {
		return index < children.size()? children.at(index) : nullptr;
	}

	bool Container::addChild(Control *child) {
		children.push_back(child);
		return true;
	}

	bool Container::removeChild(Child *to_remove) {
		for (auto iter = children.begin(); iter != children.end(); ++iter)
			if (*iter == to_remove) {
				children.erase(iter);
				to_remove->setParent(nullptr);
				return true;
			}

		return false;
	}

	Control * Container::childAtOffset(int x, int y) const {
		const Position pos = getPosition();
		x += pos.left;
		y += pos.top;

		for (Control *child: children) {
			const Position &cpos = child->getPosition();
			if ((cpos.left <= x) && (x <= cpos.right()) && (cpos.top <= y) && (y <= cpos.bottom()))
				return child;
		}

		return nullptr;
	}

	bool Container::requestResize(Control *, size_t, size_t) {
		return false;
	}

	void Container::redraw() {
		for (Control *child: children)
			child->draw();
	}
}
