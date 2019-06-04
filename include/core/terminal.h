#ifndef HAUNTED_CORE_TERMINAL_H_
#define HAUNTED_CORE_TERMINAL_H_

#include <iostream>
#include <memory>
#include <vector>

#include <termios.h>

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
			ui::control *root = nullptr;
			ui::control *focused = nullptr;

			int rows, cols;

			static termios getattr();
			static void setattr(const termios &);

			termios original;
			void apply();
			void reset();

			// signal() takes a pointer to a static function.
			// To get around this, every terminal object whose watch_size()
			// method is called adds itself to a static vector of terminal
			// pointers. When the WINCH signal handler is called, it notifies
			// all the listening terminal objects of the terminal's new
			// dimensions.
			static void winch_handler(int);
			static std::vector<terminal *> winch_targets;
			void winch(int, int);

		public:
			termios attrs;
			bool raw = false;

			terminal(std::istream &);
			terminal();

			~terminal();

			void cbreak();
			void watch_size();
			void redraw();
			void set_root(ui::control *);
			void draw();

			bool add_child(ui::control *) override;
			terminal * get_term() const override;

			operator bool() const;
			terminal & operator>>(int &);
			terminal & operator>>(char &);
			terminal & operator>>(key &);
	};
}

#endif
