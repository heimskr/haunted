#ifndef HAUNTED_UI_TEXTBOX_H_
#define HAUNTED_UI_TEXTBOX_H_

#include <deque>
#include <string>
#include <utility>

#include "ui/control.h"
#include "core/terminal.h"

namespace haunted::tests {
	class maintest;
}

namespace haunted::ui {
	/**
	 * Represents a line of text. This is a thin wrapper over a regular string that also includes a continuation column.
	 * In irssi, messages that are too wide for a single line are wrapped; each new line begins at the same column as
	 * the message did, after the timestamp and nick indicator. This wrapper makes a generalized version of that feature
	 * possible in textbox.
	 * Note that the text is assumed to contain no newlines.
	 */
	struct textline {
		/** The raw text of the line. */
		std::string text = "";

		/** The number of blank spaces at the beginning of a row to use when the line is longer than the width of its
		 *  container and has to be wrapped. The first row of the line isn't padded, but all subsequent rows are. */
		int continuation = 0;

		textline(std::string text, int continuation): text(text), continuation(continuation) {}
		textline(std::string text): text(text) {}
		textline(int continuation): continuation(continuation) {}
		textline(): textline("", 0) {}
		operator std::string() const;
		bool operator==(const textline &) const;
	};

	/**
	 * Represents a multiline box of text.
	 */
	class textbox: public virtual control {
		friend class haunted::tests::maintest;

		private:
			std::deque<textline> lines;

			/** The number of lines the container has been scrolled vertically. If -1, the textbox will always scroll to
			 *  the bottom whenever a new line is added. For values >= 0, the textbox won't scroll, even if the new text
			 *  is outside the visible region. */
			int voffset = -1;

			/** The number of columns the container has been scrolled horizontally. */
			// int hoffset = 0;

			/** Whether to wrap long lines based on the continuation column. */
			bool wrap = true;

			/** Whether margins have been set. */
			bool in_margins = false;

			/** Empties the buffer and replaces it with 0-continuation lines from a vector of string. */
			void set_lines(const std::vector<std::string> &);

			/** When a new line is added, it's usually not necessary to completely redraw the component. Instead,
			 *  scrolling the component and printing only the new line is sufficient. */
			void draw_new_line(const textline &);

			/** Returns the number of rows on the terminal a line of text would occupy. */
			size_t line_rows(const textline &line) const;

			/** Returns the total number of rows occupied by all the lines in the text box. */
			size_t total_rows() const;

			/** Returns a pair of the line at a given row (ignoring voffset and zero-based) and the number of rows past
			 *  the start of the line. For example, if the textbox contains one line that occupies a single row and a
			 *  second line that spans 5 rows, then calling this function with 4 will return {lines[1], 3}. */
			std::pair<textline &, size_t> line_at_row(size_t);

			/** Clears the region of the screen occupied by the textbox. */
			void clear();

			/** Returns the string to print on a given row (zero-based) of the textbox. Handles text wrapping and
			 *  scrolling automatically. */
			std::string text_at_row(size_t);

		public:
			/** Constructs a textbox with a parent, a position and initial contents. */
			textbox(container *parent, position pos, const std::vector<std::string> &contents);

			/** Constructs a textbox with a parent and position and empty contents. */
			textbox(container *parent, position pos): textbox(parent, pos, {}) {}

			/** Constructs a textbox with a parent, initial contents and a default position. */
			textbox(container *parent, const std::vector<std::string> &contents);

			/** Constructs a textbox with a parent, a default position and empty contents. */
			textbox(container *parent): textbox(parent, std::vector<std::string> {}) {}

			/** Deletes all lines in the textbox. */
			void clear_lines();

			/** Scrolls the textbox down (positive argument) or up (negative argument). */
			void vscroll(int);

			/** Returns the vertical offset. */
			int get_voffset() const;

			/** Returns the effective vertical offset, accounting for forced scrolling (voffset = -1). */
			int effective_voffset() const;

			/** Sets the vertical offset. */
			void set_voffset(int);

			/** Draws the textbox on the terminal. */
			void draw();

			/** Adds a string to the end of the textbox. */
			textbox & operator+=(const std::string &);
			/** Adds a line to the end of the textbox. */
			textbox & operator+=(const textline &);

			/** Returns the textbox's contents. */
			operator std::string() const;

			using iterator = std::deque<textline>::iterator;
			using reverse_iterator = std::deque<textline>::reverse_iterator;
			using const_iterator = std::deque<textline>::const_iterator;
			using const_reverse_iterator = std::deque<textline>::const_reverse_iterator;

			              iterator   begin() { return lines.begin();   }
			      reverse_iterator  rbegin() { return lines.rbegin();  }
			        const_iterator  cbegin() { return lines.cbegin();  }
			const_reverse_iterator crbegin() { return lines.crbegin(); }
			              iterator     end() { return lines.end();     }
			      reverse_iterator    rend() { return lines.rend();    }
			        const_iterator    cend() { return lines.cend();    }
			const_reverse_iterator   crend() { return lines.crend();   }
			size_t size() const { return lines.size(); }
	};
}

#endif
