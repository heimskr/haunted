#ifndef HAUNTED_CORE_TERMINAL_H_
#define HAUNTED_CORE_TERMINAL_H_

#include <iostream>
#include <memory>
#include <termios.h>

#include "core/key.h"

namespace haunted {
	/**
	 * This class enables interaction with terminals. It uses termios to change terminal modes.
	 * When the destructor is called, it resets the modes to their original values.
	 */
	class terminal {
		private:
			std::istream &in_stream;
			static termios getattr();
			static void setattr(const termios &);

			termios original;
			void apply();
			void reset();

		public:
			termios attrs;
			bool raw = false;

			terminal(std::istream &);
			terminal();

			~terminal();

			void cbreak();

			operator bool() const;
			terminal & operator>>(int &);
			terminal & operator>>(char &);
			terminal & operator>>(key &);
	};
}

#endif
