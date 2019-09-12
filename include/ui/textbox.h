#ifndef HAUNTED_UI_TEXTBOX_H_
#define HAUNTED_UI_TEXTBOX_H_

#include <deque>
#include <string>
#include <utility>

#include "ui/colored.h"
#include "ui/control.h"
#include "core/terminal.h"
#include "core/util.h"

namespace haunted::tests {
	class maintest;
}

namespace haunted::ui {
	/**
	 * Represents a line of text. This is a base class that includes a continuation column; subclasses are responsible
	 * for producing the actual text. (Note that the text is assumed to contain no newlines.)
	 * In irssi, messages that are too wide for a single line are wrapped; each new line begins at the same column as
	 * the message did, after the timestamp and nick indicator. This wrapper makes a generalized version of that feature
	 * possible in textbox.
	 */
	struct textline {
		/** The number of blank spaces at the beginning of a row to use when the line is longer than the width of its
		 *  container and has to be wrapped. The first row of the line isn't padded, but all subsequent rows are. */
		int continuation = 0;

		textline(int continuation_): continuation(continuation_) {}

		virtual ~textline() = default;

		/** Returns the text for a given row relative to the line for a given textbox width. */
		virtual std::string text_at_row(size_t width, int row) const = 0;

		/** Returns the number of rows the line will occupy for a given width. */
		virtual int num_rows(int width) const = 0;

		/** Returns the raw text of the line. */
		virtual operator std::string() const = 0;
	};

	/**
	 * Represents a line of text with static, unchanging content.
	 */
	struct simpleline: public textline {
		/** The raw text of the line. */
		std::string text = "";

		simpleline(const std::string &text_, int continuation_ = 0);
		simpleline(int continuation_): textline(continuation_) {}
		simpleline(): simpleline("", 0) {}

		virtual std::string text_at_row(size_t width, int row) const override;
		virtual int num_rows(int width) const override;

		virtual operator std::string() const override { return text; }
		virtual bool operator==(const simpleline &) const;
	};

	/**
	 * Represents a multiline box of text.
	 */
	class textbox: public control, public colored {
		friend class haunted::tests::maintest;
		using line_ptr = std::shared_ptr<textline>;

		private:
			std::deque<line_ptr> lines;

			/** The number of lines the container has been scrolled vertically. If -1, the textbox will always scroll to
			 *  the bottom whenever a new line is added. For values >= 0, the textbox won't scroll, even if the new text
			 *  is outside the visible region. */
			int voffset = 0;

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

			/** Returns the row on which the next line should be drawn or -1 if it's out of bounds. */
			int next_row() const;

			/** Returns a pair of the line at a given row (ignoring voffset and zero-based) and the number of rows past
			 *  the start of the line. For example, if the textbox contains one line that occupies a single row and a
			 *  second line that spans 5 rows, then calling this function with 4 will return {lines[1], 3}. */
			std::pair<line_ptr, int> line_at_row(int);

			/** Clears the region of the screen occupied by the textbox. */
			void clear();

			/** Returns the string to print on a given row (zero-based) of the textbox. Handles text wrapping and
			 *  scrolling automatically. */
			std::string text_at_row(int);

		public:
			/** Constructs a textbox with a parent, a position and initial contents. */
			textbox(container *parent_, position pos_, const std::vector<std::string> &contents_);

			/** Constructs a textbox with a parent and position and empty contents. */
			textbox(container *parent_, position pos_): textbox(parent_, pos_, {}) {}

			/** Constructs a textbox with a parent, initial contents and a default position. */
			textbox(container *parent_, const std::vector<std::string> &contents_);

			/** Constructs a textbox with a parent, a default position and empty contents. */
			textbox(container *parent_): textbox(parent_, std::vector<std::string> {}) {}

			/** Constructs a textbox with no parent and no contents. */
			textbox(): textbox(nullptr, std::vector<std::string> {}) {}

			/** Deletes all lines in the textbox. */
			void clear_lines();

			/** Scrolls the textbox down (positive argument) or up (negative argument). */
			void vscroll(int = 1);

			/** Returns the vertical offset. */
			int get_voffset() const;

			/** Returns the effective vertical offset, accounting for forced scrolling (voffset = -1). */
			int effective_voffset() const;

			/** Sets the vertical offset. */
			void set_voffset(int);

			/** Returns the number of rows on the terminal a line of text would occupy. */
			int line_rows(const textline &) const;

			/** Returns the total number of rows occupied by all the lines in the text box. */
			int total_rows() const;

			/** Draws the textbox on the terminal. */
			void draw() override;

			/** Handles keyboard input. */
			bool on_key(const key &) override;

			virtual bool can_draw() const override;

			/** Adds a string to the end of the textbox. */
			textbox & operator+=(const std::string &);
			/** Adds a line to the end of the textbox. */
			textbox & operator+=(line_ptr);

			template <EXTENDS(T, textline)>
			textbox & operator+=(const T &line) {
				std::shared_ptr<T> line_copy = std::make_shared<T>(line);
				lines.push_back(line_copy);
				draw_new_line(*line_copy);
				return *this;
			}

			/** Returns the textbox's contents. */
			operator std::string() const;

			virtual terminal * get_terminal() override { return term; }
			virtual container * get_parent() const override { return parent; }

			size_t size() const { return lines.size(); }

			friend void swap(textbox &left, textbox &right);
	};
}

#endif
