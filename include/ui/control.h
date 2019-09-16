#ifndef HAUNTED_UI_CONTROL_H_
#define HAUNTED_UI_CONTROL_H_

#include "../core/hdefs.h"
#include "../core/key.h"
#include "child.h"
#include "container.h"
#include "control.h"
#include "keyhandler.h"

namespace haunted::ui {
	/**
	 * Represents a control.
	 * This includes things like boxes, text views and text inputs.
	 */
	class control: public virtual keyhandler, public child {
		protected:
			terminal *term;
			std::string name;
			bool in_margins = false;
			haunted::position pos;

			/** Sets the margins if needed, executes a function and resets the margins if needed. Returns true if the
			 *  margins were set. */
			bool try_margins(std::function<void()>);

		public:
			control() = delete;
			control(const control &) = delete;
			control & operator=(const control &) = delete;

			control(container *parent_, haunted::position pos_): child(parent_), term(nullptr), pos(pos_) {
				if (parent_ != nullptr)
					term = parent_->get_terminal();
			}

			control(const haunted::position &pos_): child(nullptr), term(nullptr), pos(pos_) {}
			control(container *parent_, terminal *term_): child(parent_), term(term_) {}
			control(container *parent_): control(parent_, parent_ == nullptr? nullptr : parent_->get_terminal()) {}

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

			virtual container * get_parent() const { return parent; }
			virtual terminal * get_terminal() { return term; }
			void set_terminal(terminal *term_) { term = term_;  }

			/** Returns the control's position. */
			haunted::position get_position() const { return pos; }

			/** Moves the cursor on the screen to the top-left corner of the control. */
			void jump();

			/** Erases the portion of the display that this control occupies. */
			virtual void clear_rect();

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

			/** Sets the terminal's scrollable region with DECSLRM to fit the control horizontally. */
			void set_hmargins();

			/** Resets the scrollable region. */
			void reset_margins();

			friend class container;

			friend void swap(control &left, control &right);
	};
}

#endif
