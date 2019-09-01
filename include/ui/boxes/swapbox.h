#ifndef HAUNTED_UI_BOXES_SWAPBOX_H_
#define HAUNTED_UI_BOXES_SWAPBOX_H_

#include <list>

#include "core/defs.h"
#include "ui/boxes/box.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted::ui::boxes {
	/**
	 * Represents a box that contains some number of controls. At most one is active at any given time; the others are
	 * kept in memory but aren't drawn.
	 */
	template <typename T>
	class swapbox: public virtual box {
		using iterator = typename std::list<T *>::iterator;

		protected:
			std::list<T *> controls = {};
			T *active = nullptr;

		public:
			swapbox<T>(container *, const position &, std::initializer_list<T *> = {});
			virtual ~swapbox() override;

			virtual bool add_child(T *);

			void set_active(T *);
			T * get_active() { return active; }

			iterator begin() { return controls.begin(); }
			iterator   end() { return controls.end();   }
			size_t  size() const { return controls.size();  }
			size_t count() const { return controls.count(); }
			bool   empty() const { return controls.empty(); }

			virtual int max_children() const override { return 1; }
			virtual void resize(const position &) override;
			virtual void draw() override;
	};
}

#endif
