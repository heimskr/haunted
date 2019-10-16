#ifndef HAUNTED_UI_CONTAINER_H_
#define HAUNTED_UI_CONTAINER_H_

#include <deque>

#include "haunted/core/defs.h"
#include "haunted/core/util.h"
#include "haunted/ui/inputhandler.h"

namespace haunted {
	class terminal;
}

namespace haunted::ui {
	class control;
	class child;

	/**
	 * Containers contain controls.
	 */
	class container: public virtual inputhandler {
		friend class control;

		public:
			using type = std::deque<control *>;

		protected:
			type children;
		
		public:
			virtual ~container();

			/** Adds a child to the container. Returns true if successful. */
			virtual bool add_child(control *);

			/** Removes a child from the container. Returns true if successful. */
			virtual bool remove_child(child *);

			/** Returns the largest number of children the container can accommodate.
			 *  A value of -1 means the container can hold an arbitrary number of children. */
			virtual int max_children() const { return -1; }

			type & get_children() { return children; }

			virtual position get_position() const = 0;

			/** Returns the terminal associated with the container. */
			virtual haunted::terminal * get_terminal() = 0;

			/** Returns the child control at an offset from the top left corner of this container. */
			virtual control * child_at_offset(int x, int y) const;

			/** Asks the parent to assign a new size to one of its children. Useful for expandoboxes.
			 *  Returns true if the request was granted. */
			virtual bool request_resize(control *, size_t width, size_t height);

			/** Redraws all the container's children in order. */
			virtual void redraw();

			bool empty()   const { return children.empty(); }
			size_t size()  const { return children.size();  }
			size_t count() const { return children.size();  }

			BEGIN_END(children)

			/** Returns the nth child (indexes `children`). */
			control * operator[](size_t);
	};
}

#endif

