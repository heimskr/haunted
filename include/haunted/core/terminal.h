#ifndef HAUNTED_CORE_TERMINAL_H_
#define HAUNTED_CORE_TERMINAL_H_

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#include <termios.h>

#include "haunted/core/Key.h"
#include "haunted/core/Mouse.h"
#include "haunted/ui/Coloration.h"
#include "haunted/ui/Container.h"

#include "lib/formicine/ansi.h"
#include "lib/formicine/performance.h"

namespace Haunted {
	/**
	 * This class enables interaction with terminals. It uses termios to change terminal modes.
	 * When the destructor is called, it resets the modes to their original values.
	 */
	class Terminal: public UI::Container {
		private:
			std::istream &inStream;
			std::mutex outputMutex;
			std::mutex winchMutex;
			std::recursive_mutex renderMutex;
			std::thread inputThread;
			termios original;

			MouseMode mmode = MouseMode::None;

			UI::Control *root = nullptr;

			// Input is sent to the focused control.
			UI::Control *focused = nullptr;

			int rows, cols;

			/** Applies the attributes in `attrs` to the terminal. */
			virtual void apply();

			/** Restores the terminal attributes to what they were before any changes were applied. */
			virtual void reset();

			/** Repeatedly reads from the terminal in a loop and dispatches the key presses to the focused control. */
			virtual void workInput();

			/** Handles window resizes. */
			virtual void winch(int, int);

			// signal() takes a pointer to a static function. To get around this, every terminal object whose
			// watch_size() method is called adds itself to a static vector of terminal pointers. When the WINCH signal
			// handler is called, it notifies all the listening terminal objects of the terminal's new dimensions.

			/** Notifies terminal objects of a window resize. */
			static void winchHandler(int);
			static std::vector<Terminal *> winchTargets;

			/** Returns the terminal attributes from tcgetaddr. */
			static termios getattr();

			/** Sets the terminal attributes with tcsetaddr. */
			static void setattr(const termios &);

		public:
			termios attrs;
			bool raw = false;
			bool suppressOutput = false;
			ansi::ansistream &outStream;
			UI::Coloration colors;

			bool dragging = false;
			MouseButton dragButton = MouseButton::Left;

			/** Called after a key is pressed and processed. */
			std::function<void(const Key &)> keyPostlistener {};

			/** Called after a mouse event is processed. */
			std::function<void(const MouseReport &)> mousePostlistener {};

			/** Called when the client receives ^c. If this returns true, the client will quit. */
			std::function<bool()> onInterrupt {[]() { return true; }};

			Terminal(std::istream &, ansi::ansistream &);
			Terminal(std::istream &in_stream): Terminal(in_stream, ansi::out) {}
			Terminal(): Terminal(std::cin) {}

			Terminal(const Terminal &) = delete;

			/** Resets terminal attributes and joins threads as necessary. */
			virtual ~Terminal();

			/** Activates cbreak mode. */
			virtual void cbreak();

			/** Sets a handler to respond to SIGWINCH signals. */
			virtual void watchSize();

			/** Redraws the entire screen if a root control exists. This also adjusts the size and position of the root
			 *  control to match the terminal. */
			virtual void redraw() override;

			/** Resets the colors to the terminal's defaults. */
			virtual void resetColors();

			/** Sets the terminal's root control. If the new root isn't the same as the old root and the `delete_old`
			 *  parameter is `true`, this function deletes the old root. */
			virtual void setRoot(UI::Control *, bool delete_old = true);
			
			/** Draws the root control if one exists. */
			virtual void draw();
			
			/** Sends a key press to whichever control is most appropriate and willing to receive it.
			 *  Returns a pointer to the control or container that ended up handling the key press. */
			virtual UI::InputHandler * sendKey(const Key &);

			virtual UI::InputHandler * sendMouse(const MouseReport &);

			/** Handles key combinations common to most console programs. */
			virtual bool onKey(const Key &) override;

			/** Starts the input-reading thread. */
			virtual void startInput();

			/** Joins all the terminal's threads. */
			virtual void join();

			/** Flushes the output stream. */
			virtual void flush();

			/** Focuses a control. */
			virtual void focus(UI::Control *);
			/** Returns the focused control. If none is currently selected, this function focuses the root control. */
			virtual UI::Control * getFocused();

			/** Adding a child to the terminal the normal way does nothing. This is so that controls whose parents don't
			 *  exist yet can use the terminal as the parent without being set as the root. The program using this
			 *  library is responsible for passing the intended root control to setRoot. */
			virtual bool addChild(UI::Control *) override;

			/** Returns the terminal. Required by Haunted::UI::container. */
			virtual Terminal * getTerminal() override { return this; }

			/** Returns true if a given control is the focused control. */
			virtual bool hasFocus(const UI::Control *) const;

			/** Returns the height (in rows) of the terminal. */
			virtual int getRows() const { return rows; }
			/** Returns the width (in columns) of the terminal. */
			virtual int getCols() const { return cols; }
			/** Returns a (0, 0)-based position representing the terminal. */
			virtual Position getPosition() const override;

			/** Recursively searches the all children within the terminal until a non-container control that contains
			 *  the given coordinate is found. */
			virtual UI::Control * childAtOffset(int x, int y) const override;

			/** Jumps to the focused widget. */
			virtual void jumpToFocused();

			/** Jumps to a position on the screen. */
			virtual void jump(int x, int y = -1);
			virtual void    up(size_t n = 1) { outStream.up(n);    }
			virtual void  down(size_t n = 1) { outStream.down(n);  }
			virtual void right(size_t n = 1) { outStream.right(n); }
			virtual void  left(size_t n = 1) { outStream.left(n);  }
			virtual void clearLine()  { outStream.clear_line();  }
			virtual void clearRight() { outStream.clear_right(); }
			virtual void clearLeft()  { outStream.clear_left();  }
			virtual void front() { outStream.hpos(0); }
			virtual void back()  { outStream.hpos(cols); }

			/** Makes the cursor visible. */
			virtual void show() { outStream.show(); }
			/** Makes the cursor invisible. */
			virtual void hide() { outStream.hide(); }

			/** Sets the mouse-reporting mode. */
			virtual void mouse(MouseMode);
			/** Returns the current mouse mode. */
			virtual MouseMode mouse() const { return mmode; }

			/** Scrolls the screen vertically. Negative numbers scroll up, positive numbers scroll down. */
			virtual void vscroll(int rows = 1);

			/** Sets the horizontal margins of the scrollable area. Zero-based. */
			virtual void hmargins(size_t left, size_t right);
			/** Resets the horizontal margins of the scrollable area. */
			virtual void hmargins();
			/** Enables horizontal margins. This must be called before calling hmargins. */
			virtual void enableHmargins();
			/** Disables horizontal margins. */
			virtual void disableHmargins();
			/** Sets the vertical margins of the scrollable area. Zero-based. */
			virtual void vmargins(size_t top, size_t bottom);
			/** Resets the vertical margins of the scrollable area. */
			virtual void vmargins();
			/** Sets the vertical and horizontal margins of the scrollable area. Zero-based. */
			virtual void margins(size_t top, size_t bottom, size_t left, size_t right);
			/** Resets the vertical and horizontal margins of the scrollable area. */
			virtual void margins();
			/** Enables origin mode: the home position is set to the top-left corner of the margins. */
			virtual void setOrigin();
			/** Disables origin mode. */
			virtual void resetOrigin();

			/** Returns a lock that gives the current thread exclusive permission to render components. */
			virtual std::unique_lock<std::recursive_mutex> lockRender();

			/** Returns true if in_stream is in a valid state. */
			virtual operator bool() const;
			/** Reads a single raw character from the terminal as an int. */
			virtual Terminal & operator>>(int &);
			/** Reads a single raw character from the terminal. */
			virtual Terminal & operator>>(char &);
			/** Reads a key from the terminal. This conveniently handles much of the weirdness of terminal input. */
			virtual Terminal & operator>>(Key &);

			void debugTree();

			/** Writes pretty much anything to the terminal. */
			template <typename T>
			Terminal & operator<<(const T &t) {
				auto w = formicine::perf.watch("template <T> operator<<(Terminal, T)");
				if (!suppressOutput) {
					std::unique_lock uniq(outputMutex);
					outStream << t;
				}

				return *this;
			}

			/** Deactivates a formicine style or color. */
			template <typename T>
			Terminal & operator>>(const T &t) {
				std::unique_lock uniq(outputMutex);
				outStream >> t;
				return *this;
			}
	};
}

#endif
