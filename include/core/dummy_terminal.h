#ifndef HAUNTED_CORE_DUMMY_TERMINAL_H_
#define HAUNTED_CORE_DUMMY_TERMINAL_H_

#include <termios.h>

#include "core/terminal.h"
#include "ui/control.h"

namespace haunted {
	/**
	 * Represents a virtual terminal whose output is suppressed. Useful for unit testing.
	 */
	class dummy_terminal: public terminal {
		private:
			void apply() override;
			void reset() override;
			void winch(int, int) override;
			void setattr(const termios &) override;

		public:
			void cbreak() override;
			void watch_size() override;
			void redraw() override;
			void set_root(ui::control *) override;
			void draw() override;
			void start_input() override;
			void flush() override;
			void hmargins(size_t, size_t) override;
			void hmargins() override;
			void vmargins(size_t, size_t) override;
			void vmargins() override;
			void margins(size_t, size_t, size_t, size_t) override;
			void margins() override;
			void enable_hmargins() override;
			void disable_hmargins() override;
			void set_origin() override;
			void reset_origin() override;
			operator bool() const override;
			terminal & operator>>(int &) override;
			terminal & operator>>(char &) override;
			terminal & operator>>(key &) override;
	};
}

#endif
