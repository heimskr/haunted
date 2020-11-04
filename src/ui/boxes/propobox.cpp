#include <stdexcept>

#include "haunted/core/Terminal.h"
#include "haunted/ui/boxes/PropoBox.h"

namespace Haunted::UI::Boxes {
	PropoBox::PropoBox(Container *parent_, const Position &pos_, double ratio_, BoxOrientation orientation_):
	DualBox(parent_, pos_, orientation_), ratio(ratio_) {
		if (parent_)
			parent_->addChild(this);

		if (ratio < 0)
			throw std::domain_error("Box ratio cannot be negative");
	}

	PropoBox::PropoBox(Container *parent_, double ratio_, BoxOrientation orientation_, Control *one, Control *two,
	const Position &pos_): PropoBox(parent_, pos_, ratio_, orientation_) {
		for (Control *control: {one, two}) {
			control->setParent(this);
			control->setTerminal(terminal);
			children.push_back(control);
		}
	}

	void PropoBox::setRatio(const double ratio_) {
		if (ratio != ratio_) {
			ratio = ratio_;
			resize();
		}
	}

	void PropoBox::resize(const Position &new_pos) {
		Control::resize(new_pos);

		if (children.size() == 0) {
			return;
		} else if (children.size() == 1) {
			children[0]->resize(new_pos);
		} else if (children.size() == 2) {
			if (orientation == BoxOrientation::Horizontal) {
				children[0]->resize({position.left, position.top, sizeOne(), position.height});
				children[1]->resize({position.left + sizeOne(), position.top, sizeTwo(), position.height});
			} else if (orientation == BoxOrientation::Vertical) {
				children[0]->resize({position.left, position.top, position.width, sizeOne()});
				children[1]->resize({position.left, position.top + sizeOne(), position.width, sizeTwo()});
			} else throw std::runtime_error("Unknown orientation: " + std::to_string(static_cast<int>(orientation)));
		} else throw std::runtime_error("Invalid number of children for PropoBox: " + std::to_string(children.size()));

		redraw();
	}

	void PropoBox::draw() {
		if (!canDraw())
			return;

		Colored::draw();

		auto lock = terminal->lockRender();
		for (Control *child: children)
			child->draw();
	}

	int PropoBox::sizeOne() const {
		return getSize() - sizeTwo();
	}

	int PropoBox::sizeTwo() const {
		return getSize() / (1.0 + ratio);
	}

	int PropoBox::getSize() const {
		return orientation == BoxOrientation::Horizontal? position.width : position.height;
	}
}
