#ifndef HAUNTED_CORE_DUMMYTERMINAL_H_
#define HAUNTED_CORE_DUMMYTERMINAL_H_

#include <termios.h>

#include "haunted/core/Terminal.h"
#include "haunted/ui/Control.h"

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
				suppressOutput = true;
			}

			~DummyTerminal() {}

			void cbreak() override {}
			void watchSize() override {}
			void redraw() override {}
			void setRoot(UI::Control *, bool) override {}
			void draw() override {}
			void startInput() override {}
			void flush() override {}
			void hmargins(size_t, size_t) override {}
			void hmargins() override {}
			void vmargins(size_t, size_t) override {}
			void vmargins() override {}
			void margins(size_t, size_t, size_t, size_t) override {}
			void margins() override {}
			void enableHmargins() override {}
			void disableHmargins() override {}
			void setOrigin() override {}
			void resetOrigin() override {}
			void jump(int, int) override {}
			void up(size_t) override {}
			void down(size_t) override {}
			void right(size_t) override {}
			void left(size_t) override {}
			void vscroll(int) override {}
			void clearLine() override {}
			void show() override {}
			void hide() override {}
			operator bool() const override { return true; }
			Terminal & operator>>(int &)  override { return *this; }
			Terminal & operator>>(char &) override { return *this; }
			Terminal & operator>>(Key &)  override { return *this; }
	};
}

#endif
