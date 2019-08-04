#ifndef HAUNTED_UI_CONTAINER_H_
#define HAUNTED_UI_CONTAINER_H_

#include <vector>

#include "ui/keyhandler.h"

namespace haunted {
	class terminal;
}

namespace haunted::ui {
	class control;
	class child;

	/**
	 * Containers contain controls.
	 */
	class container: public keyhandler {
		protected:
			std::vector<control *> children;
		
		public:
			virtual ~container() = 0;

			/** Adds a child to the container. Returns true if successful. */
			virtual bool add_child(control *);

			/** Removes a child from the container. Returns true if successful. */
			virtual bool remove_child(child *);

			/** Returns the largest number of children the container can accommodate.
			 *  A value of -1 means the container can hold an arbitrary number of children. */
			virtual int max_children() const { return -1; }

			/** Returns the number of children the container currently holds. */
			virtual int child_count()  const { return children.size(); }

			/** Returns the terminal associated with the container. */
			virtual haunted::terminal * get_terminal() = 0;

			/** Asks the parent to assign a new size to one of its children. Useful for expandoboxes.
			 *  Returns true if the request was granted. */
			virtual bool request_resize(control *, size_t width, size_t height);

			/** Redraws all the container's children in order. */
			virtual void redraw();

			/** Returns the nth child (indexes `children`). */
			control * operator[](size_t);
	};
}

#endif

