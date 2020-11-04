#ifndef HAUNTED_UI_CONTAINER_H_
#define HAUNTED_UI_CONTAINER_H_

#include <deque>

#include "haunted/core/Defs.h"
#include "haunted/core/Util.h"
#include "haunted/ui/InputHandler.h"

namespace Haunted {
	class Terminal;
}

namespace Haunted::UI {
	class Control;
	class Child;

	/**
	 * Containers contain controls.
	 */
	class Container: public virtual InputHandler {
		friend class Control;

		public:
			using Type = std::deque<Control *>;

		protected:
			Type children;
		
		public:
			virtual ~Container();

			/** Adds a child to the container. Returns true if successful. */
			virtual bool addChild(Control *);

			/** Removes a child from the container. Returns true if successful. */
			virtual bool removeChild(Child *);

			/** Returns the largest number of children the container can accommodate.
			 *  A value of -1 means the container can hold an arbitrary number of children. */
			virtual int maxChildren() const { return -1; }

			Type & getChildren() { return children; }

			virtual Position getPosition() const = 0;

			/** Returns the terminal associated with the container. */
			virtual Haunted::Terminal * getTerminal() = 0;

			/** Returns the child control at an offset from the top left corner of this container. */
			virtual Control * childAtOffset(int x, int y) const;

			/** Asks the parent to assign a new size to one of its children. Useful for expandoboxes.
			 *  Returns true if the request was granted. */
			virtual bool requestResize(Control *, size_t width, size_t height);

			/** Redraws all the container's children in order. */
			virtual void redraw();

			bool   empty() const { return children.empty(); }
			size_t size()  const { return children.size();  }
			size_t count() const { return children.size();  }

			BEGIN_END(children)

			/** Returns the nth child (indexes `children`). */
			Control * operator[](size_t);
	};
}

#endif

