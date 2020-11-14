#ifndef HAUNTED_UI_TEXTLINE_H_
#define HAUNTED_UI_TEXTLINE_H_

#include <deque>
#include <functional>
#include <string>
#include <vector>

#include "haunted/core/Mouse.h"

namespace Haunted::UI {
	template <template <typename... T> typename C>
	class Textbox;

	/**
	 * Represents a line of text. This is a base class that includes a continuation column; subclasses are responsible
	 * for producing the actual text. (Note that the text is assumed to contain no newlines.)
	 * In irssi, messages that are too wide for a single line are wrapped; each new line begins at the same column as
	 * the message did, after the timestamp and nick indicator. This wrapper makes a generalized version of that feature
	 * possible in textbox.
	 */
	template <template <typename... T> typename C>
	class TextLine {
		public:
			std::vector<std::string> lines_ {};
			int numRows_ = -1;
			bool dirty = true;
			bool cleaning = false;

			/** Caches the return values of num_rows and text_at_row. */
			void clean(int width) {
				if (!dirty || cleaning)
					return;

				cleaning = true;
				numRows_ = numRows(width);
				for (int row = 0; row < numRows_; ++row)
					lines_.push_back(textAtRow(width, row));

				cleaning = false;
				dirty = false;
			}

		public:
			Textbox<C> *box = nullptr;
			std::function<void(const MouseReport &)> mouseFunction;

			TextLine() {}

			virtual ~TextLine() = default;

			/** Removes all cached data and marks the TextLine as dirty. */
			void markDirty() {
				dirty = true;
				numRows_ = -1;
				lines_.clear();
			}

			/** Returns the number of blank spaces at the beginning of a row to use when the line's longer than the
			 *  width of its container and has to be wrapped. The first row of the line isn't padded, but all subsequent
			 *  rows are. */
			virtual int getContinuation() = 0;

			/** Returns the text for a given row relative to the line for a given textbox width. */
			virtual std::string textAtRow(size_t width, int row, bool pad_right = true) {
				if (!dirty) {
					return lines_[row];
				} else if (!cleaning) {
					clean(width);
					return lines_[row];
				}

				auto w = formicine::perf.watch("TextLine::textAtRow");
				const std::string text = std::string(*this);
				const size_t text_length = ansi::length(text);

				if (row == 0) {
					return text_length < width? ansi::substr(text, 0, width) + std::string(width - text_length, ' ')
						: ansi::substr(text, 0, width);
				}

				const int continuation = getContinuation();
				const size_t index = continuation + row * (width - continuation);
				if (index >= text_length)
					return pad_right? std::string(width, ' ') : "";

				std::string chunk = std::string(continuation, ' ') + ansi::substr(text, index, width - continuation);
				const size_t chunk_length = ansi::length(chunk);
				if (pad_right && chunk_length < width)
					return chunk + std::string(width - chunk_length, ' ');

				return chunk;
			}

			/** Returns the number of rows the line will occupy for a given width. */
			virtual int numRows(int width) {
				if (!dirty) {
					return numRows_;
				} else if (!cleaning) {
					clean(width);
					return numRows_;
				}

				const std::string text = ansi::strip(*this);
				// auto w = formicine::perf.watch("TextLine::numRows");

				int length = ansi::length(text);
				if (length <= width)
					return 1;

				// Ignore all the text on the first line because it's not affected by continuation.
				length -= width;

				const int continuation = getContinuation();
				const int adjusted_continuation = width - (width == continuation? continuation - 1 : continuation);
				return length / adjusted_continuation + (length % adjusted_continuation? 2 : 1);
			}

			/** Called when the line is clicked on. 
			 *  The MouseReport's position is relative to the top left of the line. */
			virtual void onMouse(const MouseReport &report) { if (mouseFunction) mouseFunction(report); }

			/** Returns the raw text of the line. */
			virtual operator std::string() = 0;

			bool operator==(TextLine &other) {
				return getContinuation() == other.getContinuation() && std::string(*this) == std::string(other);
			}
	};

	using DequeLine  = TextLine<std::deque>;
	using VectorLine = TextLine<std::vector>;
}

#endif
