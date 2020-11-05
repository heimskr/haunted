#ifndef HAUNTED_UI_CHILD_H_
#define HAUNTED_UI_CHILD_H_

namespace Haunted::UI {
	class Container;

	/**
	 * A child is anything that has a parent. This includes controls and all containers except terminals.
	 */
	class Child {
		protected:
			Container *parent;

		public:
			Child(Container *parent_ = nullptr): parent(parent_) {}
			Child(const Child &) = delete;
			Child & operator=(const Child &) = delete;

			virtual ~Child() = 0;

			Container * getParent();
			virtual void setParent(Container *);

			friend void swap(Child &left, Child &right);
	};
}

#endif
