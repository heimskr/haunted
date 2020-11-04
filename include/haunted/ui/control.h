#ifndef HAUNTED_UI_CONTROL_H_
#define HAUNTED_UI_CONTROL_H_

#include "haunted/core/Defs.h"
#include "haunted/core/Key.h"
#include "haunted/ui/Child.h"
#include "haunted/ui/Container.h"
#include "haunted/ui/InputHandler.h"

namespace Haunted::UI {
	/**
	 * Represents a control.
	 * This includes things like boxes, text views and text inputs.
	 */
	class Control: public virtual InputHandler, public Child {
		protected:
			/** The control's controlling terminal. */
			Terminal *terminal;

			/** A name (ideally unique) that identifies the control. */
			std::string name;

			/** Whether margins are currently set for this component. */
			bool inMargins = false;

			/** The absolute position of the control on the screen. */
			Haunted::Position position = {};

			/** Sets the margins if needed, executes a function and resets the margins if needed. Returns true if the
			 *  margins were set. */
			bool tryMargins(std::function<void()>);

		public:
			/** Whether to ignore this control when calculating the indices of the other children of this control's
			 *  parent. */
			bool ignoreIndex = false;

			Control() = delete;
			Control(const Control &) = delete;
			Control & operator=(const Control &) = delete;

			Control(Container *parent_, const Haunted::Position &position_);
			Control(const Haunted::Position &position_): Child(nullptr), terminal(nullptr), position(position_) {}
			Control(Container *parent_, Terminal *terminal_);
			Control(Container *parent_);

			virtual ~Control() = 0;

			/** Returns the control's identifier. */
			virtual std::string getID(bool pad = false) const;

			/** Sets the control's name. */
			virtual Control & setName(const std::string &name_) { name = name_; return *this; }
			
			/** Gets the control's name. */
			virtual const std::string & getName() { return name; }

			/** Renders the control on the terminal. */
			virtual void draw() = 0;

			/** Returns whether the control's in a state in which it can be rendered. */
			virtual bool canDraw() const;

			/** Resizes the control to fit a new position. */
			virtual void resize(const Haunted::Position &);

			/** Reassigns the control's current position to itself. Useful for container. */
			virtual void resize();

			/** Moves the control to a given coordinate. */
			virtual void move(int left, int top);

			/** Focuses the control. */
			virtual void focus();

			/** Sets the parent and adopts its terminal. */
			virtual void setParent(Container *) override;

			virtual Container * getParent() const { return parent; }
			virtual Terminal * getTerminal() { return terminal; }
			void setTerminal(Terminal *terminal_) { terminal = terminal_;  }

			/** Returns the control's position. */
			virtual Position getPosition() const { return position; }

			/** Moves the cursor on the screen to the top-left corner of the control. */
			virtual void jump();

			/** Jumps to an appropriate location within the control. Useful for textinput. */
			virtual void jumpFocus();

			/** Erases the portion of the display that this control occupies. */
			virtual void clearRect();

			/** Flushes the terminal's output buffer. */
			void flush();

			/** Returns true if the control is its terminal's focused control. */
			bool hasFocus() const;

			/** Returns true if the control's right edge is at the right edge of the screen. */
			bool atRight() const;

			/** Returns true if the control's left edge is at the left edge of the screen. */
			bool atLeft() const;

			/** Sets the terminal's scrollable region with DECSLRM and DECSTBM to fit the control. */
			void setMargins();

			/** Sets the terminal's scrollable region with DECSLRM to fit the control horizontally. */
			void setHmargins();

			/** Resets the scrollable region. */
			void resetMargins();

			/** Returns the control's index within its parent's children. If the control has no parent or if it somehow
			 *  turned out not to be present among its parent's children, this function returns -1. */
			ssize_t getIndex() const;

			friend class Container;

			friend void swap(Control &left, Control &right);
	};
}

#endif
