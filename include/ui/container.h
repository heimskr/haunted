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

			virtual bool add_child(control *);
			virtual bool remove_child(child *);
			virtual int max_children() const;
			virtual int child_count() const;
			virtual haunted::terminal * get_terminal() = 0;

			/** Returns the nth child (indexes `children`). */
			control * operator[](size_t);
	};
}

#endif

