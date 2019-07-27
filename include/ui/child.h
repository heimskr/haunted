#ifndef HAUNTED_UI_CHILD_H_
#define HAUNTED_UI_CHILD_H_

#include "ui/container.h"

namespace haunted::ui {
	/**
	 * A child is anything that has a parent. This includes controls and all containers except terminals.
	 */
	class child {
		protected:
			container *parent;
			child(container *parent): parent(parent) {}

		public:
			container * get_parent();
	};
}

#endif
