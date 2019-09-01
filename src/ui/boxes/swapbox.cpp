#include "ui/boxes/swapbox.h"

namespace haunted::ui::boxes {
	template <typename T>
	swapbox<T>::swapbox(container *parent_, const position &pos_, std::initializer_list<T *> controls_):
	box(parent_, pos_), controls(controls_) {
		if (parent_)
			parent_->add_child(this);

		for (T *ctrl: controls_)
			ctrl->set_parent(nullptr);

		if (controls_.size() != 0) {
			active = *controls_.begin();
			active->set_parent(this);
			active->set_terminal(term);
		}
	}

	template <typename T>
	swapbox<T>::~swapbox() {
		children.clear();
		for (T *ctrl: controls)
			delete ctrl;
	}


// Public instance methods


	template <typename T>
	bool swapbox<T>::add_child(control *) {
		throw std::runtime_error("add_child() isn't allowed for swapbox; use add_control() instead.");
	}

	template <typename T>
	void swapbox<T>::set_active(T *new_active) {
		if (new_active == active)
			return;

		children.clear();
		if (new_active == nullptr) {
			active->set_parent(nullptr);
			active->set_terminal(nullptr);
			active = nullptr;

			clear_rect();
		} else if (new_active != nullptr) {
			if (active != nullptr) {
				active->set_parent(nullptr);
				active->set_terminal(nullptr);
			}

			active = new_active;
			children.push_back(active);

			active->set_parent(this);
			active->set_terminal(term);
			active->draw();
		}
	}

	template <typename T>
	void swapbox<T>::resize(const position &new_pos) {
		control::resize(new_pos);
		if (active)
			active->resize(new_pos);
	}

	template <typename T>
	void swapbox<T>::draw() {
		if (!can_draw())
			return;

		if (active)
			active->draw();
		else
			clear_rect();
	}
}
