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

		public:

			child(container *parent = nullptr): parent(parent) {
				DBGFN();
				DBG("child::child(parent=" << parent <<"): setting parent of " << this << " to " << parent);
			}
			container * get_parent();
			virtual void set_parent(container *);
	};
}

#endif
