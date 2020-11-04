#include "haunted/core/Defs.h"
#include "haunted/core/Terminal.h"
#include "haunted/ui/boxes/SwapBox.h"

namespace Haunted::UI::Boxes {
	SwapBox::SwapBox(Container *parent_, const Position &pos_, std::initializer_list<Control *> children_):
	Box(parent_, pos_) {
		if (parent_)
			parent_->addChild(this);

		for (Control *control: children_) {
			control->setParent(this);
			control->setTerminal(nullptr);
			children.push_back(control);
		}

		if (children_.size() != 0) {
			active = *children_.begin();
			active->setParent(this);
			active->setTerminal(terminal);
		}
	}


// Public instance methods


	void SwapBox::setActive(Control *new_active) {
		// To prevent children's terminals from disappearing mid-render, we need to acquire the terminal's render lock.
		std::unique_lock<std::recursive_mutex> lock;
		if (terminal)
			lock = terminal->lockRender();

		if (new_active == active)
			return;

		if (new_active == nullptr) {
			active->setTerminal(nullptr);
			active = nullptr;

			clearRect();
		} else if (new_active != nullptr) {
			if (active != nullptr) {
				active->setTerminal(nullptr);
				if (active->getPosition() != new_active->getPosition())
					new_active->resize(active->getPosition());
			}

			active = new_active;
			active->setTerminal(terminal);
			active->setParent(this);
			active->draw();
		}

		if (std::find(children.begin(), children.end(), new_active) == children.end())
			children.push_back(new_active);
	}

	Control * SwapBox::childAtOffset(int x, int y) const {
		return (x - position.left < position.width && y - position.top < position.height)? active : nullptr;
	}

	void SwapBox::resize(const Position &new_pos) {
		Control::resize(new_pos);
		if (active)
			active->resize(new_pos);
	}

	void SwapBox::draw() {
		if (!canDraw())
			return;

		if (active)
			active->draw();
		else
			clearRect();
	}

	bool SwapBox::onKey(const Key &key) {
		return keyFunction? keyFunction(key) : (active && active->onKey(key));
	}
}
