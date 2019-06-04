#include "lib/ansi.h"

#include "core/terminal.h"
#include "ui/control.h"

namespace haunted::ui {
	control::~control() = default;

	void control::resize(const haunted::position &new_pos) {
		// It's up to the caller of resize() to also call draw().
		pos = new_pos;
	}

	void control::jump() {
		pos.jump();
	}

	void control::clear_rect() {
		// There are four different ways to clear text on a row: erase the entire row, erase
		// everything to the left of the cursor, erase everything to the right of the cursor and
		// simply writing spaces. The first is ideal, the next two are okay and the last is the
		// worst option. Which option we can use depends on the left-offset and width of the
		// control.
		ansi::save();
		if (pos.left == 0) {
			// If we're at the left edge of the screen, we can clear-line if the width is full, or
			// clear-left otherwise.
			if (pos.width == term->get_cols()) {
				for (int i = 1; i <= pos.height; ++i) {
					ansi::jump(pos.top + i, 1);
					ansi::clear_line();
				}
			} else {
				for (int i = 1; i <= pos.height; ++i) {
					ansi::jump(pos.top + i, pos.width + 1);
					ansi::clear_left();
				}
			}
		} else if (pos.left + pos.width == term->get_cols()) {
			// If we're at the right edge of the screen, we can clear-right.
			for (int i = 1; i <= pos.height; ++i) {
				ansi::jump(pos.top + i, pos.left + 1);
				ansi::clear_right();
			}
		} else {
			// If the control doesn't reach either end of the screen, we have to print a bunch
			// of spaces.
			std::string spaces(pos.width, ' ');
			for (int i = 1; i <= pos.height; ++i) {
				ansi::jump(pos.top + i, pos.left + 1);
				std::cout << spaces;
			}
		}

		ansi::restore();
	}

	void control::focus() {
		term->focus(this);
	}
}
