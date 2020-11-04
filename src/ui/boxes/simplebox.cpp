#include "haunted/core/Terminal.h"
#include "haunted/ui/boxes/SimpleBox.h"

namespace Haunted::UI::Boxes {
	void SimpleBox::resize(const Position &pos) {
		Control::resize(pos);
		if (!children.empty())
			children.at(0)->move(pos.left, pos.top);
	}

	void SimpleBox::move(int left, int top) {
		Control::move(left, top);
		if (!children.empty())
			children.at(0)->move(left, top);
	}

	void SimpleBox::draw() {
		if (canDraw() && !children.empty()) {
			auto lock = terminal->lockRender();
			children.at(0)->draw();
		}
	}
}
