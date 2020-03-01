#include "haunted/core/terminal.h"
#include "haunted/ui/boxes/simplebox.h"

namespace Haunted::UI::Boxes {
	void simplebox::resize(const position &pos) {
		control::resize(pos);
		if (!children.empty())
			children.at(0)->move(pos.left, pos.top);
	}

	void simplebox::move(int left, int top) {
		control::move(left, top);
		if (!children.empty())
			children.at(0)->move(left, top);
	}

	void simplebox::draw() {
		if (can_draw() && !children.empty()) {
			auto lock = term->lock_render();
			children.at(0)->draw();
		}
	}
}
