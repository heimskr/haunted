#ifndef HAUNTED_UI_TEXTLINE_H_
#define HAUNTED_UI_TEXTLINE_H_

#include <functional>
#include <string>
#include <vector>

#include "haunted/core/Mouse.h"

namespace Haunted::UI {
	class Textbox;

	/**
	 * Represents a line of text. This is a base class that includes a continuation column; subclasses are responsible
	 * for producing the actual text. (Note that the text is assumed to contain no newlines.)
	 * In irssi, messages that are too wide for a single line are wrapped; each new line begins at the same column as
	 * the message did, after the timestamp and nick indicator. This wrapper makes a generalized version of that feature
	 * possible in textbox.
	 */
	class TextLine {
		public:
			std::vector<std::string> lines_ {};
			int numRows_ = -1;
			bool dirty = true;
			bool cleaning = false;

			/** Caches the return values of num_rows and text_at_row. */
			void clean(int width);

		public:
			Textbox *box = nullptr;
			std::function<void(const MouseReport &)> mouseFunction;

			TextLine() {}

			virtual ~TextLine() = default;

			/** Removes all cached data and marks the TextLine as dirty. */
			void markDirty();

			/** Returns the number of blank spaces at the beginning of a row to use when the line's longer than the
			 *  width of its container and has to be wrapped. The first row of the line isn't padded, but all subsequent
			 *  rows are. */
			virtual int getContinuation() = 0;

			/** Returns the text for a given row relative to the line for a given textbox width. */
			virtual std::string textAtRow(size_t width, int row, bool pad_right = true);

			/** Returns the number of rows the line will occupy for a given width. */
			virtual int numRows(int width);

			/** Called when the line is clicked on. 
			 *  The MouseReport's position is relative to the top left of the line. */
			virtual void onMouse(const MouseReport &report) { if (mouseFunction) mouseFunction(report); }

			/** Returns the raw text of the line. */
			virtual operator std::string() = 0;

			bool operator==(TextLine &);
	};
}

#endif
