#include "haunted/core/defs.h"
#include "haunted/core/terminal.h"
#include "haunted/ui/boxes/swapbox.h"

namespace Haunted::UI::Boxes {
	SwapBox::SwapBox(container *parent_, const position &pos_, std::initializer_list<Control *> children_):
	Box(parent_, pos_) {
		if (parent_)
			parent_->add_child(this);

		for (Control *ctrl: children_) {
			ctrl->set_parent(this);
			ctrl->set_terminal(nullptr);
			children.push_back(ctrl);
		}

		if (children_.size() != 0) {
			active = *children_.begin();
			active->set_parent(this);
			active->set_terminal(term);
		}
	}


// Public instance methods


	void SwapBox::set_active(Control *new_active) {
		// To prevent children's terminals from disappearing mid-render, we need to acquire the terminal's render lock.
		std::unique_lock<std::recursive_mutex> lock;
		if (term)
			lock = term->lock_render();

		if (new_active == active)
			return;

		if (new_active == nullptr) {
			active->set_terminal(nullptr);
			active = nullptr;

			clear_rect();
		} else if (new_active != nullptr) {
			if (active != nullptr) {
				active->set_terminal(nullptr);
				if (active->get_position() != new_active->get_position())
					new_active->resize(active->get_position());
			}

			active = new_active;
			active->set_terminal(term);
			active->set_parent(this);
			active->draw();
		}

		if (std::find(children.begin(), children.end(), new_active) == children.end())
			children.push_back(new_active);
	}

	Control * SwapBox::child_at_offset(int x, int y) const {
		return (x - pos.left < pos.width && y - pos.top < pos.height)? active : nullptr;
	}

	void SwapBox::resize(const position &new_pos) {
		Control::resize(new_pos);
		if (active)
			active->resize(new_pos);
	}

	void SwapBox::draw() {
		if (!can_draw())
			return;

		if (active)
			active->draw();
		else
			clear_rect();
	}

	bool SwapBox::on_key(const key &k) {
		return key_fn? key_fn(k) : active && active->on_key(k);
	}
}
