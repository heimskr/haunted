#ifndef HAUNTED_CORE_TERMINAL_H_
#define HAUNTED_CORE_TERMINAL_H_

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <termios.h>

#include "haunted/core/key.h"
#include "haunted/core/mouse.h"
#include "haunted/ui/coloration.h"
#include "haunted/ui/container.h"
#include "haunted/ui/control.h"

#include "lib/formicine/ansi.h"
#include "lib/formicine/performance.h"

namespace haunted {
	/**
	 * This class enables interaction with terminals. It uses termios to change terminal modes.
	 * When the destructor is called, it resets the modes to their original values.
	 */
	class terminal: public ui::container {
		private:
			std::istream &in_stream;
			std::mutex output_mutex;
			std::mutex winch_mutex;
			std::recursive_mutex render_mutex;
			std::thread input_thread;
			termios original;

			mouse_mode mmode = mouse_mode::none;

			ui::control *root = nullptr;

			// Input is sent to the focused control.
			ui::control *focused = nullptr;

			int rows, cols;

			/** Applies the attributes in `attrs` to the terminal. */
			virtual void apply();

			/** Restores the terminal attributes to what they were before any changes were applied. */
			virtual void reset();

			/** Repeatedly reads from the terminal in a loop and dispatches the key presses to the focused control. */
			virtual void work_input();

			/** Handles window resizes. */
			virtual void winch(int, int);

			// signal() takes a pointer to a static function. To get around this, every terminal object whose
			// watch_size() method is called adds itself to a static vector of terminal pointers. When the WINCH signal
			// handler is called, it notifies all the listening terminal objects of the terminal's new dimensions.

			/** Notifies terminal objects of a window resize. */
			static void winch_handler(int);
			static std::vector<terminal *> winch_targets;

			/** Returns the terminal attributes from tcgetaddr. */
			static termios getattr();

			/** Sets the terminal attributes with tcsetaddr. */
			static void setattr(const termios &);

		public:
			termios attrs;
			bool raw = false;
			bool suppress_output = false;
			ansi::ansistream &out_stream;
			ui::coloration colors;

			bool dragging = false;
			mouse_button drag_button = mouse_button::left;

			/** Called after a key is pressed and processed. */
			std::function<void(const key &)> key_postlistener {};

			/** Called when the client receives ^c. If this returns true, the client will quit. */
			std::function<bool()> on_interrupt {[]() { return true; }};

			terminal(std::istream &, ansi::ansistream &);
			terminal(std::istream &in_stream): terminal(in_stream, ansi::out) {}
			terminal(): terminal(std::cin) {}

			terminal(const terminal &) = delete;

			/** Resets terminal attributes and joins threads as necessary. */
			virtual ~terminal();

			/** Activates cbreak mode. */
			virtual void cbreak();

			/** Sets a handler to respond to SIGWINCH signals. */
			virtual void watch_size();

			/** Redraws the entire screen if a root control exists. This also adjusts the size and position of the root
			 *  control to match the terminal. */
			virtual void redraw() override;

			/** Resets the colors to the terminal's defaults. */
			virtual void reset_colors();

			/** Sets the terminal's root control. If the new root isn't the same as the old root and the `delete_old`
			 *  parameter is `true`, this function deletes the old root. */
			virtual void set_root(ui::control *, bool delete_old = true);
			
			/** Draws the root control if one exists. */
			virtual void draw();
			
			/** Sends a key press to whichever control is most appropriate and willing to receive it.
			 *  Returns a pointer to the control or container that ended up handling the key press. */
			virtual ui::keyhandler * send_key(const key &);

			virtual /* ui::mousehandler * */ void send_mouse(const mouse_report &);

			/** Handles key combinations common to most console programs. */
			virtual bool on_key(const key &) override;

			/** Starts the input-reading thread. */
			virtual void start_input();

			/** Joins all the terminal's threads. */
			virtual void join();

			/** Flushes the output stream. */
			virtual void flush();

			/** Focuses a control. */
			virtual void focus(ui::control *);
			/** Returns the focused control. If none is currently selected, this function focuses the root control. */
			virtual ui::control * get_focused();

			/** Adding a child to the terminal the normal way does nothing. This is so that controls whose parents
			 *  don't exist yet can use the terminal as the parent without being set as the root. The program using this
			 *  library is responsible for passing the intended root control to set_root. */
			virtual bool add_child(ui::control *) override;

			/** Returns the terminal. Required by haunted::ui::container. */
			virtual terminal * get_terminal() override { return this; }

			/** Returns true if a given control is the focused control. */
			virtual bool has_focus(const ui::control *) const;

			/** Returns the height (in rows) of the terminal. */
			virtual int get_rows() const { return rows; }
			/** Returns the width (in columns) of the terminal. */
			virtual int get_cols() const { return cols; }
			/** Returns a (0, 0)-based position representing the terminal. */
			virtual position get_position() const;

			/** Jumps to the focused widget. */
			virtual void jump_to_focused();

			/** Jumps to a position on the screen. */
			virtual void jump(int x, int y = -1);
			virtual void    up(size_t n = 1) { out_stream.up(n);    }
			virtual void  down(size_t n = 1) { out_stream.down(n);  }
			virtual void right(size_t n = 1) { out_stream.right(n); }
			virtual void  left(size_t n = 1) { out_stream.left(n);  }
			virtual void clear_line() { out_stream.clear_line(); }

			/** Makes the cursor visible. */
			virtual void show() { out_stream.show(); }
			/** Makes the cursor invisible. */
			virtual void hide() { out_stream.hide(); }

			/** Sets the mouse-reporting mode. */
			virtual void mouse(mouse_mode);

			/** Scrolls the screen vertically. Negative numbers scroll up, positive numbers scroll down. */
			virtual void vscroll(int rows = 1);

			/** Sets the horizontal margins of the scrollable area. Zero-based. */
			virtual void hmargins(size_t left, size_t right);
			/** Resets the horizontal margins of the scrollable area. */
			virtual void hmargins();
			/** Enables horizontal margins. This must be called before calling hmargins. */
			virtual void enable_hmargins();
			/** Disables horizontal margins. */
			virtual void disable_hmargins();
			/** Sets the vertical margins of the scrollable area. Zero-based. */
			virtual void vmargins(size_t top, size_t bottom);
			/** Resets the vertical margins of the scrollable area. */
			virtual void vmargins();
			/** Sets the vertical and horizontal margins of the scrollable area. Zero-based. */
			virtual void margins(size_t top, size_t bottom, size_t left, size_t right);
			/** Resets the vertical and horizontal margins of the scrollable area. */
			virtual void margins();
			/** Enables origin mode: the home position is set to the top-left corner of the margins. */
			virtual void set_origin();
			/** Disables origin mode. */
			virtual void reset_origin();

			/** Returns a lock that gives the current thread exclusive permission to render components. */
			virtual std::unique_lock<std::recursive_mutex> lock_render();

			/** Returns true if in_stream is in a valid state. */
			virtual operator bool() const;
			/** Reads a single raw character from the terminal as an int. */
			virtual terminal & operator>>(int &);
			/** Reads a single raw character from the terminal. */
			virtual terminal & operator>>(char &);
			/** Reads a key from the terminal. This conveniently handles much of the weirdness of terminal input. */
			virtual terminal & operator>>(key &);

			void debug_tree();

			/** Writes pretty much anything to the terminal. */
			template <typename T>
			terminal & operator<<(const T &t) {
				auto w = formicine::perf.watch("template <T> operator<<(terminal, T)");
				if (!suppress_output) {
					std::unique_lock uniq(output_mutex);
					out_stream << t;
				}

				return *this;
			}

			/** Deactivates a formicine style or color. */
			template <typename T>
			terminal & operator>>(const T &t) {
				std::unique_lock uniq(output_mutex);
				out_stream >> t;
				return *this;
			}
	};
}

#endif
