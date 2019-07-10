#ifndef HAUNTED_CORE_TERMINAL_H_
#define HAUNTED_CORE_TERMINAL_H_

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <termios.h>

#include "formicine/ansi.h"

#include "core/key.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted {
	/**
	 * This class enables interaction with terminals. It uses termios to change terminal modes.
	 * When the destructor is called, it resets the modes to their original values.
	 */
	class terminal: public virtual ui::container {
		private:
			std::istream &in_stream;
			ansi::ansistream out_stream;
			std::mutex render_mx;
			std::thread input_thread;
			termios original;

			ui::control *root = nullptr;

			// Input is sent to the focused control.
			ui::control *focused = nullptr;

			int rows, cols;

			/** Applies the attributes in `attrs` to the terminal. */
			void apply();

			/** Restores the terminal attributes to what they were before any changes were applied. */
			void reset();

			/** Repeatedly reads from the terminal in a loop and dispatches the key presses to the focused control. */
			void work_input();
			
			/** Sends a key press to whichever control is most appropriate and willing to receive it.
			 *  Returns a pointer to the control or container that ended up handling the key press. */
			ui::keyhandler * send_key(key);

			/** Handles window resizes. */
			void winch(int, int);

			/** Returns the terminal attributes from tcgetaddr. */
			static termios getattr();

			/** Sets the terminal attributes with tcsetaddr. */
			static void setattr(const termios &);

			// signal() takes a pointer to a static function. To get around this, every terminal object whose
			// watch_size() method is called adds itself to a static vector of terminal pointers. When the WINCH signal
			// handler is called, it notifies all the listening terminal objects of the terminal's new dimensions.

			/** Notifies terminal objects of a window resize. */
			static void winch_handler(int);
			static std::vector<terminal *> winch_targets;

		public:
			termios attrs;
			bool raw = false;

			terminal(std::istream &, ansi::ansistream);
			terminal(std::istream &in_stream): terminal(in_stream, ansi::ansistream()) {}
			terminal(): terminal(std::cin) {}

			~terminal();

			/** Activates cbreak mode. */
			void cbreak();

			/** Sets a handler to respond to SIGWINCH signals. */
			void watch_size();

			/** Redraws the entire screen if a root control exists. This also adjusts the size and position of the root
			 *  control to match the terminal. */
			void redraw();

			/** Sets the terminal's root control. If the new root isn't the same as the old root, this function deletes
			 *  the old root. */
			void set_root(ui::control *);
			
			/** Draws the root control if one exists. */
			void draw();

			/** Starts the input-reading thread. */
			void start_input();

			/** Flushes the output stream. */
			void flush();

			/** Focuses a control. */
			void focus(ui::control *);
			/** Returns the focused control. If none is currently selected, this function focuses the root control. */
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

			/** Sets the horizontal margins of the scrollable area. Zero-based. */
			void hmargins(size_t, size_t);
			/** Resets the horizontal margins of the scrollable area. */
			void hmargins();
			/** Sets the vertical margins of the scrollable area. Zero-based. */
			void vmargins(size_t, size_t);
			/** Resets the vertical margins of the scrollable area. */
			void vmargins();
			/** Sets the vertical and horizontal margins of the scrollable area. Zero-based. */
			void margins(size_t top, size_t bottom, size_t left, size_t right);
			/** Resets the vertical and horizontal margins of the scrollable area. */
			void margins();
			void enable_hmargins();
			void disable_hmargins();
			/** Enables origin mode: the home position is set to the top-left corner of the margins. */
			void set_origin();
			/** Disables origin mode. */
			void reset_origin();

			/** Returns true if in_stream is in a valid state. */
			operator bool() const;
			/** Reads a single raw character from the terminal as an int. */
			terminal & operator>>(int &);
			/** Reads a single raw character from the terminal. */
			terminal & operator>>(char &);
			/** Reads a key from the terminal. This conveniently handles much of the weirdness of terminal input. */
			terminal & operator>>(key &);

			/** Writes pretty much anything to the terminal. */
			template <typename T>
			terminal & operator<<(const T &t) {
				out_stream << t;
				return *this;
			}

			/** Returns whether the parent terminal is iTerm. */
			static bool is_iterm();
	};
}

#endif
