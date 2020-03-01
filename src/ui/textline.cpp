#include "haunted/ui/textline.h"
#include "lib/formicine/performance.h"

namespace haunted::ui {


// Protected instance methods


	void textline::mark_dirty() {
		dirty = true;
		num_rows_ = -1;
		lines_.clear();
	}

	void textline::clean(int width) {
		if (!dirty)
			return;

		cleaning = true;

		num_rows_ = num_rows(width);
		for (int row = 0; row < num_rows_; ++row)
			lines_.push_back(text_at_row(width, row));

		cleaning = false;
		dirty = false;
	}


// Public instance methods


	std::string textline::text_at_row(size_t width, int row, bool pad_right) {
		if (!dirty) {
			return lines_[row];
		} else if (!cleaning) {
			clean(width);
			return lines_[row];
		}

		auto w = formicine::perf.watch("textline::text_at_row");
		const std::string text = std::string(*this);
		const size_t text_length = ansi::length(text);

		if (row == 0) {
			return text_length < width? ansi::substr(text, 0, width) + std::string(width - text_length, ' ')
			     : ansi::substr(text, 0, width);
		}

		const int continuation = get_continuation();
		const size_t index = continuation + row * (width - continuation);
		if (index >= text_length)
			return pad_right? std::string(width, ' ') : "";

		std::string chunk = std::string(continuation, ' ') + ansi::substr(text, index, width - continuation);
		const size_t chunk_length = ansi::length(chunk);
		if (pad_right && chunk_length < width)
			return chunk + std::string(width - chunk_length, ' ');

		return chunk;
	}

	int textline::num_rows(int width) {
		if (!dirty) {
			return num_rows_;
		} else if (!cleaning) {
			clean(width);
			return num_rows_;
		}

		const std::string text = ansi::strip(*this);
		// auto w = formicine::perf.watch("textline::num_rows");

		int length = ansi::length(text);
		if (length <= width)
			return 1;

		// Ignore all the text on the first line because it's not affected by continuation.
		length -= width;

		const int continuation = get_continuation();
		const int adjusted_continuation = width - (width == continuation? continuation - 1 : continuation);
		return length / adjusted_continuation + (length % adjusted_continuation? 2 : 1);
	}

	bool textline::operator==(textline &other) {
		return get_continuation() == other.get_continuation() && std::string(*this) == std::string(other);
	}
}
