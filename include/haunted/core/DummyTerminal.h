#ifndef HAUNTED_CORE_DUMMYTERMINAL_H_
#define HAUNTED_CORE_DUMMYTERMINAL_H_

#include <termios.h>

#include "haunted/core/terminal.h"
#include "haunted/ui/control.h"

namespace Haunted {
	/**
	 * Represents a virtual terminal whose output is suppressed. Useful for unit testing.
	 */
	class DummyTerminal: public Terminal {
		private:
			void apply() override {}
			void reset() override {}
			void winch(int, int) override {}

		public:
			DummyTerminal() {
				suppress_output = true;
			}

			~DummyTerminal() {}

			void cbreak() override {}
			void watch_size() override {}
			void redraw() override {}
			void set_root(UI::control *, bool) override {}
			void draw() override {}
			void start_input() override {}
			void flush() override {}
			void hmargins(size_t, size_t) override {}
			void hmargins() override {}
			void vmargins(size_t, size_t) override {}
			void vmargins() override {}
			void margins(size_t, size_t, size_t, size_t) override {}
			void margins() override {}
			void enable_hmargins() override {}
			void disable_hmargins() override {}
			void set_origin() override {}
			void reset_origin() override {}
			void jump(int, int) override {}
			void up(size_t) override {}
			void down(size_t) override {}
			void right(size_t) override {}
			void left(size_t) override {}
			void vscroll(int) override {}
			void clear_line() override {}
			void show() override {}
			void hide() override {}
			operator bool() const override { return true; }
			Terminal & operator>>(int &)  override { return *this; }
			Terminal & operator>>(char &) override { return *this; }
			Terminal & operator>>(key &)  override { return *this; }
	};
}

#endif
