#ifndef HAUNTED_UI_CONTAINER_H_
#define HAUNTED_UI_CONTAINER_H_

#include <vector>

namespace haunted {
	class terminal;
}

namespace haunted::ui {
	class control;

	/**
	 * Containers contain controls.
	 */
	class container {
		protected:
			std::vector<control *> children;
		
		public:
			virtual ~container() = 0;

			virtual bool add_child(control *);
			virtual int max_children() const;
			virtual int child_count() const;
			virtual haunted::terminal * get_term() = 0;

			control * operator[](size_t);
	};
}

#endif

