#ifndef HAUNTED_UI_TEXTBOX_H_
#define HAUNTED_UI_TEXTBOX_H_

#include <string>
#include <deque>

#include "ui/control.h"
#include "core/terminal.h"

namespace haunted::ui {
	/**
	 * Represents a line of text. This is a thin wrapper over a regular string that also includes a continuation column.
	 * In irssi, messages that are too wide for a single line are wrapped; each new line begins at the same column as
	 * the message did, after the timestamp and nick indicator. This wrapper makes a generalized version of that feature
	 * possible in textbox.
	 */
	struct textline {
		std::string text = "";
		int continuation = 0;
		textline(std::string text, int continuation): text(text), continuation(continuation) {}
		textline(std::string text): text(text) {}
		textline(int continuation): continuation(continuation) {}
		textline(): textline("", 0) {}
	};

	/**
	 * Represents a multiline box of text.
	 */
	class textbox: public virtual control {
		private:
			std::deque<textline> lines;

			/** The number of lines the container has been scrolled. If -1, the textbox will always scroll to the bottom 
			 *  whenever a new line is added. For values >= 0, the textbox won't scroll, even if the new text is outside
			 *  the visible region. */
			int offset;

			/** Empties the buffer and replaces it with 0-continuation lines from a vector of string. */
			void set_lines(const std::vector<std::string> &);

			/** When a new line is added, it's usually not necessary to completely redraw the component. Instead,
			 *  scrolling the component and printing only the new line is sufficient. */
			void draw_new_line(const textline &);

		public:
			/** Constructs a textbox with a parent and a position and initial contents and scroll offset. */
			textbox(container *parent, position pos, const std::vector<std::string> &contents, int offset_);

			textbox(container *parent, position pos, const std::vector<std::string> &contents):
				textbox(parent, pos, contents, -1) {}

			/** Constructs a textbox with a parent and position and a default buffer and cursor. */
			textbox(container *parent, position pos): textbox(parent, pos, {}) {}

			/** Constructs a textbox with a parent, initial content and a default position and scroll offset. */
			textbox(container *parent, const std::vector<std::string> &contents, int offset_);

			/** Constructs a textbox with a parent, initial contents and a default position and scroll offset. */
			textbox(container *parent, const std::vector<std::string> &contents): textbox(parent, contents, 0) {}

			/** Constructs a textbox with a parent and a default position, contents and scroll offset. */
			textbox(container *parent): textbox(parent, std::vector<std::string> {}) {}

			void clear();
			void draw();

			textbox & operator+=(const std::string &);
			textbox & operator+=(const textline &);
			operator std::string() const;
	};
}

#endif
