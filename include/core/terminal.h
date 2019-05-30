#ifndef HAUNTED_TERMINAL_H_
#define HAUNTED_TERMINAL_H_

#include <iostream>
#include <memory>
#include <termios.h>

#include "haunted/key.h"

namespace haunted {
	/**
	 * This class enables interaction with terminals.
	 * It makes use of termios to change terminal modes.
	 * With its destructor, it resets 
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

			terminal(std::istream &);
			terminal();

			~terminal();

			void cbreak();

			operator bool() const;
			terminal & operator>>(char &);
	};
}

#endif
