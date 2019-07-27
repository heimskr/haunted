#include "ui/boxes/box.h"

namespace haunted::ui::boxes {
	void box::resize(const position &pos) {
		control::resize(pos);
		if (!children.empty())
			children.at(0)->move(pos.left, pos.top);
	}

	void box::move(int left, int top) {
		control::move(left, top);
		if (!children.empty())
			children.at(0)->move(left, top);
	}
}
