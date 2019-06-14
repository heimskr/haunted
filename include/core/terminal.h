#ifndef HAUNTED_CORE_TERMINAL_H_
#define HAUNTED_CORE_TERMINAL_H_

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <termios.h>

#include "lib/ansi.h"

#include "core/key.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted {
	/**
	 * This class enables interaction with terminals.
	 * It uses termios to change terminal modes.
	 * When the destructor is called, it resets
	 * the modes to their original values.
	 */
	class terminal: public virtual ui::container {
		private:
			std::istream &in_stream;
			formicine::ansistream out_stream;
			std::mutex render_mx;
			std::thread input_thread;

			ui::control *root = nullptr;

			// Input is sent to the focused control.
			ui::control *focused = nullptr;

			int rows, cols;

			static termios getattr();
			static void setattr(const termios &);

			termios original;

			/** Applies the attributes in `attrs` to the terminal. */
			void apply();

			/** Restores the original terminal attributes. */
			void reset();

			/** Reads and dispatches key presses in a loop. */
			void work_input();
			
			/** Sends a key press to whichever control is most appropriate and willing to receive
			 *  it. Returns a pointer to the control or container that ended up handling the key
			 *  press. */
			ui::keyhandler * send_key(key);

			// signal() takes a pointer to a static function.
			// To get around this, every terminal object whose watch_size()
			// method is called adds itself to a static vector of terminal
			// pointers. When the WINCH signal handler is called, it notifies
			// all the listening terminal objects of the terminal's new
			// dimensions.
			static void winch_handler(int);
			static std::vector<terminal *> winch_targets;

			/** Handles window resizes. */
			void winch(int, int);

		public:
			termios attrs;
			bool raw = false;

			terminal(std::istream &, formicine::ansistream);
			terminal(std::istream &in_stream): terminal(in_stream, formicine::ansistream()) {}
			terminal(): terminal(std::cin) {}

			~terminal();

			/** Activates cbreak mode. */
			void cbreak();

			/** Sets a handler to respond to SIGWINCH signals. */
			void watch_size();

			/** Redraws the entire screen if a root control exists. This also adjusts the size and
			 *  position of the root control to match the terminal. */
			void redraw();

			/** Sets the terminal's root control. If the new root isn't the same as the old root,
			 *  this function deletes the old root. */
			void set_root(ui::control *);
			
			/** Draws the root control if one exists. */
			void draw();

			/** Starts the input-reading thread. */
			void start_input();

			/** Flushes the output stream. */
			void flush();

			/** Focuses a control. */
			void focus(ui::control *);
			/** Returns the focused control. If none is currently selected, * this function focuses
			 *  the root control. */
			ui::control * get_focused();

			/** Adds a child to the terminal by setting its root to the child. */
			bool add_child(ui::control *) override;

			/** Returns the terminal. Required by haunted::ui::container. */
			terminal * get_term() override;

			/** Returns true if a given control is the focused control. */
			bool has_focus(const ui::control *) const;

			/** Returns the height (in rows) of the terminal. */
			int get_rows() const;
			/** Returns the width (in columns) of the terminal. */
			int get_cols() const;

			/** Returns true if in_stream is in a valid state. */
			operator bool() const;
			/** Reads a single raw character from the terminal as an int. */
			terminal & operator>>(int &);
			/** Reads a single raw character from the terminal. */
			terminal & operator>>(char &);
			/** Reads a key from the terminal. This conveniently handles much of the weirdness of
			 *  terminal input. */
			terminal & operator>>(key &);

			/** Writes pretty much anything to the terminal. */
			template <typename T>
			terminal & operator<<(const T &t) {
				out_stream << t;
				return *this;
			}
	};
}

#endif
