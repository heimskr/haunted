#ifndef HAUNTED_UI_CHILD_H_
#define HAUNTED_UI_CHILD_H_

#include "haunted/ui/container.h"

namespace haunted::ui {
	/**
	 * A child is anything that has a parent. This includes controls and all containers except terminals.
	 */
	class child {
		protected:
			container *parent;

		public:
			child(container *parent_ = nullptr): parent(parent_) {}
			child(const child &) = delete;
			child & operator=(const child &) = delete;

			virtual ~child() = 0;

			container * get_parent();
			virtual void set_parent(container *);

			friend void swap(child &left, child &right);
	};
}

#endif
