#include "haunted/ui/Textbox.h"
#include "haunted/ui/TextLine.h"
#include "lib/formicine/performance.h"

namespace Haunted::UI {


// Public instance methods


	void TextLine::markDirty() {
		dirty = true;
		numRows_ = -1;
		lines_.clear();
	}

	void TextLine::clean(int width) {
		if (!dirty || cleaning)
			return;

		cleaning = true;
		numRows_ = numRows(width);
		for (int row = 0; row < numRows_; ++row)
			lines_.push_back(textAtRow(width, row));

		cleaning = false;
		dirty = false;
	}

	std::string TextLine::textAtRow(size_t width, int row, bool pad_right) {
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

	int TextLine::numRows(int width) {
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

	bool TextLine::operator==(TextLine &other) {
		return getContinuation() == other.getContinuation() && std::string(*this) == std::string(other);
	}
}
