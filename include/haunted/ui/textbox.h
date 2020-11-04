#ifndef HAUNTED_UI_TEXTBOX_H_
#define HAUNTED_UI_TEXTBOX_H_

#include <deque>
#include <functional>
#include <string>
#include <utility>

#include "haunted/ui/ColoredControl.h"
#include "haunted/core/Terminal.h"
#include "haunted/core/Util.h"

#include "haunted/ui/TextLine.h"
#include "haunted/ui/SimpleLine.h"

#include "lib/formicine/performance.h"

namespace Haunted::Tests {
	class maintest;
}

namespace Haunted::UI {
	/**
	 * Represents a multiline box of text.
	 */
	class Textbox: public ColoredControl {
		friend class Haunted::Tests::maintest;

		public:
			using LinePtr = std::shared_ptr<TextLine>;

		protected:
			/** Holds all the textlines in the box. */
			std::deque<LinePtr> lines;

			/** The number of rows the container has been scrolled vertically. */
			int voffset = 0;

			/** Whether the textbox should automatically scroll to keep up with lines added to the bottom. */
			bool autoscroll = false;

			/** Empties the buffer and replaces it with 0-continuation lines from a vector of string. */
			void setLines(const std::vector<std::string> &);

			/** When a new line is added, it's usually not necessary to completely redraw the component. Instead,
			 *  scrolling the component and printing only the new line is sufficient.
			 *  @param inserted Whether the line has already been inserted into the textbox's collection. */
			void drawNewLine(TextLine &, bool inserted = false);

			/** Returns the row on which the next line should be drawn or -1 if it's out of bounds. */
			int nextRow(int offset_offset = 0);

			/** Returns a pair of the line at a given row (ignoring voffset and zero-based) and the number of rows past
			 *  the start of the line. For example, if the textbox contains one line that occupies a single row and a
			 *  second line that spans 5 rows, then calling this function with 4 will return {lines[1], 3}. */
			std::pair<TextLine *, int> lineAtRow(int);

			/** Returns the string to print on a given row (zero-based) of the textbox. Handles text wrapping and
			 *  scrolling automatically. */
			std::string textAtRow(int, bool pad_right = true);

			/** Performs vertical scrolling for a given number of rows if autoscrolling is enabled and the right
			 *  conditions are met. This should be done after the line is added to the set of lines but before the line
			 *  is drawn. Returns true if this method caused any scrolling.*/
			bool doScroll(size_t rows);

		public:
			/** The cached return value of total_rows(). */
			int totalRows_ = -1;

			/** Marks the cached return value of total_rows() as dirty. */
			void rowsDirty();

			/** Marks the num_rows_ values of the contained lines as dirty. */
			void linesDirty();

			/** Marks everything as dirty. */
			void markDirty();

		public:
			/** The minimum number of lines that must be visible at the top. */
			unsigned int scroll_buffer = 0;

			/** Constructs a textbox with a parent, a position and initial contents. */
			Textbox(Container *parent_, const Position &pos_, const std::vector<std::string> &contents_);

			/** Constructs a textbox with a parent and position and empty contents. */
			Textbox(Container *parent_, const Position &pos_): Textbox(parent_, pos_, {}) {}

			/** Constructs a textbox with a parent, initial contents and a default position. */
			Textbox(Container *parent_, const std::vector<std::string> &contents_);

			/** Constructs a textbox with a parent, a default position and empty contents. */
			Textbox(Container *parent_): Textbox(parent_, std::vector<std::string>()) {}

			/** Constructs a textbox with no parent and no contents. */
			Textbox(): Textbox(nullptr, std::vector<std::string>()) {}

			/** Deletes all lines in the textbox. */
			void clearLines();

			std::deque<LinePtr> & getLines() { return lines; }

			/** Scrolls the textbox down (positive argument) or up (negative argument). */
			void vscroll(int = 1);

			/** Returns the vertical offset. */
			int getVoffset() const;

			/** Sets the vertical offset. */
			void setVoffset(int);

			bool getAutoscroll() const { return autoscroll; }

			void setAutoscroll(bool);

			/** Returns the number of rows on the terminal a line of text would occupy. */
			int lineRows(TextLine &);

			/** Returns the total number of rows occupied by all the lines in the text box. */
			int totalRows();

			/** Draws the textbox on the terminal. */
			void draw() override;

			/** Resizes the textbox to fit a new position. */
			void resize(const Haunted::Position &) override;

			/** Handles keyboard input. */
			bool onKey(const Key &) override;

			/** Calls the clicked textline's onMouse method. */
			bool onMouse(const MouseReport &) override;

			/** Handles textbox-related keyboard input. */
			bool defaultOnKey(const Key &);

			bool canDraw() const override;

			void focus() override;

			/** Adds a string to the end of the textbox. */
			Textbox & operator+=(const std::string &);
			
			/** Adds a line to the end of the textbox. */
			template <EXTENDS(T, TextLine)>
			Textbox & operator+=(T &line) {
				auto w = formicine::perf.watch("template textbox::operator+=");
				std::unique_ptr<T> line_copy = std::make_unique<T>(line);
				line_copy->box = this;
				if (autoscroll)
					doScroll(line_copy->numRows(position.width));
				lines.push_back(std::move(line_copy));
				rowsDirty();
				drawNewLine(*lines.back(), true);
				return *this;
			}

			/** Returns the textbox's contents. */
			operator std::string();

			virtual Terminal * getTerminal() override { return terminal; }
			virtual Container * getParent() const override { return parent; }

			size_t size() const { return lines.size(); }

			friend void swap(Textbox &left, Textbox &right);
	};
}

#endif
