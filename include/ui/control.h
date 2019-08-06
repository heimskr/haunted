#ifndef HAUNTED_UI_CONTROL_H_
#define HAUNTED_UI_CONTROL_H_

#include "core/defs.h"
#include "core/key.h"
#include "child.h"
#include "container.h"
#include "control.h"
#include "keyhandler.h"

namespace haunted::ui {
	/**
	 * Represents a control.
	 * This includes things like boxes, text views and text inputs.
	 */
	class control: public virtual keyhandler, public virtual child {
		protected:
			terminal *term;
			std::string name;
			haunted::position pos;

		public:
			control(container *parent, const haunted::position &pos):
				child(parent), term(parent == nullptr? nullptr : parent->get_terminal()), pos(pos) {}
			control(const haunted::position &pos): child(nullptr), term(nullptr), pos(pos) {}
			control(container *parent, terminal *term): child(parent), term(term) {}
			control(container *parent):
				control(parent, parent == nullptr? nullptr : parent->get_terminal()) {}
			control(): control(nullptr, nullptr) {}

			virtual ~control() = 0;

			/** Returns the control's identifier. */
			virtual std::string get_id(bool pad = false) const;

			/** Sets the control's name. */
			virtual control & set_name(const std::string &name_) { name = name_; return *this; }
			
			/** Renders the control on the terminal. */
			virtual void draw() = 0;

			/** Returns whether the control's in a state in which it can be rendered. */
			virtual bool can_draw() const;

			/** Resizes the control to fit a new position. */
			virtual void resize(const haunted::position &);

			/** Reassigns the control's current position to itself. Useful for container. */
			virtual void resize();

			/** Moves the control to a given coordinate. */
			virtual void move(int left, int top);

			/** Focuses the control. */
			virtual void focus();

			/** Sets the parent and adopts its terminal. */
			virtual void set_parent(container *) override;

			terminal & get_terminal() const;
			void set_terminal(terminal *);
			void set_terminal(terminal &);

			/** Returns the control's position. */
			haunted::position get_position() const;

			/** Moves the cursor on the screen to the top-left corner of the control. */
			void jump();

			/** Erases the portion of the display that this control occupies. */
			void clear_rect();

			/** Flushes the terminal's output buffer. */
			void flush();

			/** Returns true if the control is its terminal's focused control. */
			bool has_focus() const;

			/** Returns true if the control's right edge is at the right edge of the screen. */
			bool at_right() const;

			/** Returns true if the control's left edge is at the left edge of the screen. */
			bool at_left() const;

			/** Sets the terminal's scrollable region with DECSLRM and DECSTBM to fit the control. */
			void set_margins();

			/** Resets the scrollable region. */
			void reset_margins();

			friend class container;
	};
}

#endif
