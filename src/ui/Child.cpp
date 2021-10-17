#include "haunted/ui/Container.h"
#include "haunted/ui/Child.h"

namespace Haunted::UI {
	Container * Child::getParent() {
		return parent;
	}

	void Child::setParent(Container *new_parent) {
		if (parent == new_parent)
			return;

		if (parent)
			parent->removeChild(this);

		parent = new_parent;
	}

	void swap(Child &left, Child &right) {
		std::swap(left.parent, right.parent);
	}
}
