#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "ui/textbox.h"
#include "formicine/ansi.h"
#include "tests/test.h"

namespace haunted::ui {
	bool textline::operator==(const textline &other) const {
		return continuation == other.continuation && std::string(*this) == std::string(other);
	}

	simpleline::simpleline(const std::string &text_, int continuation_): textline(continuation_), text(text_) {
		text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
		text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
	}

	std::string textline::text_at_row(size_t width, int row) const {
		const std::string text = std::string(*this);
		const size_t text_length = ansi::length(text);

		if (row == 0) {
			return text_length < width? ansi::substr(text, 0, width) + std::string(width - text_length, ' ')
			     : ansi::substr(text, 0, width);
		}

		const size_t index = continuation + row * (width - continuation);
		if (index >= text_length)
			return std::string(width, ' ');

		std::string chunk = std::string(continuation, ' ') + ansi::substr(text, index, width - continuation);
		const size_t chunk_length = ansi::length(chunk);
		if (chunk_length < width)
			return chunk + std::string(width - chunk_length, ' ');

		return chunk;
	}

	int textline::num_rows(int width) const {
		const std::string text = ansi::strip(*this);

		int length = ansi::length(text);
		if (length <= width)
			return 1;

		// Ignore all the text on the first line because it's not affected by continuation.
		length -= width;

		const int adjusted_continuation = width - (width == continuation? continuation - 1 : continuation);
		return length / adjusted_continuation + (length % adjusted_continuation? 2 : 1);
	}

	bool simpleline::operator==(const simpleline &other) const {
		return continuation == other.continuation && text == other.text;
	}

	std::ostream & operator<<(std::ostream &os, const simpleline &line) {
		os << line.text;
		return os;
	}

	textbox::textbox(container *parent_, position pos_, const std::vector<std::string> &contents):
	control(parent_, pos_) {
		if (parent_ != nullptr)
			parent_->add_child(this);
		set_lines(contents);
		// add_child could modify the position, so we set the position again at the end to overoverwrite it.
		pos = pos_;
	}

	textbox::textbox(container *parent_, const std::vector<std::string> &contents): control(parent_) {
		if (parent_ != nullptr)
			parent_->add_child(this);
		set_lines(contents);
	}


// Private instance methods


	void textbox::set_lines(const std::vector<std::string> &strings) {
		lines.clear();
		for (const std::string &str: strings) {
			std::unique_ptr<simpleline> ptr = std::make_unique<simpleline>(str, 0);
			lines.push_back(std::move(ptr));
		}
	}

	void textbox::draw_new_line(const textline &line, bool inserted) {
		if (!can_draw())
			return;

		int new_lines = line_rows(line);

		// We need to subtract one if the new line is already in the buffer.
		int next = next_row() - (inserted? new_lines : 0);
		if (voffset != -1 && next < 0)
			return;


		set_margins();
		in_margins = true;
		colored::draw();

		if (voffset == -1 && pos.height < total_rows())
			term->vscroll(-new_lines);

		term->jump(0, next);
		for (int row = next, i = 0; row < pos.height && i < new_lines; ++row, ++i) {
			if (i > 0)
				*term << "\n";
			*term << line.text_at_row(pos.width, i);
		}

		reset_margins();
		in_margins = false;
	}

	int textbox::next_row() const {
		int offset = effective_voffset();
		int total = total_rows();

		// Return -1 if the next row is below the visible area.
		if (pos.height <= total - offset)
			return -1;

		return total - offset;
	}

	std::pair<textline *, int> textbox::line_at_row(int row) {
		if (lines.empty() || row >= total_rows())
			throw std::out_of_range("Invalid row index: " + std::to_string(row));

		if (!wrap)
			return {lines[row].get(), 0};

		int line_count = lines.size(), index = 0, row_count = 0, last_count = 0, offset = -1;

		for (index = 0, row_count = 0; index < line_count; ++index) {
			last_count = line_rows(*lines[index]);
			if (row_count <= row && row < row_count + last_count) {
				offset = row - row_count;
				break;
			} else {
				row_count += last_count;
			}
		}
		
		if (line_count <= index)
			throw std::out_of_range("Line index too large: " + std::to_string(index));

 		return {lines[index].get(), offset == -1? row - row_count : offset};
	}

	void textbox::clear() {
		if (!can_draw())
			return;

		bool should_reset_margins = false;
		if (!in_margins) {
			set_margins();
			should_reset_margins = true;
		}

		for (int i = 0; i < pos.height; ++i) {
			if (i != 0)
				term->out_stream.down();
			term->out_stream.delete_chars(pos.width);
		}

		if (should_reset_margins)
			reset_margins();
	}

	std::string textbox::text_at_row(int row) {
		const size_t cols = pos.width;
		
		textline *line;
		size_t offset;

		if (pos.height <= row || row < 0)
			return "";

		try {
			std::tie(line, offset) = line_at_row(row + effective_voffset());
		} catch (std::out_of_range &) {
			return std::string(cols, ' ');
		}

		const std::string line_text(*line);
		const int continuation = line->continuation;

		if (offset == 0) {
			const size_t line_length = ansi::length(line_text);
			return line_length <= cols? line_text + std::string(cols - line_length, ' ')
				: ansi::substr(line_text, 0, cols);
		}

		// Number of chars visible per row on a continued line
		size_t continuation_chars = cols - line->continuation;

		// Ignore the first line. 
		std::string str = ansi::substr(line_text, cols);

		// Erase the continued lines after the first line and before the continuation at the given row.
		ansi::erase(str, 0, (offset - 1) * continuation_chars);

		// Erase all characters after the continuation if any remain.
		if (continuation_chars < ansi::length(str))
			ansi::erase(str, continuation_chars, std::string::npos);

		const size_t str_length = ansi::length(str);
		// Return the continuation padding plus the segment of text visible on the row.
		if (continuation + str_length < cols)
			return std::string(continuation, ' ') + str + std::string(cols - continuation - str_length, ' ');
		return std::string(continuation, ' ') + str;
	}


// Public instance methods


	void textbox::clear_lines() {
		lines.clear();
		if (0 < voffset)
			voffset = 0;
		draw();
	}

	void textbox::vscroll(int delta) {
		const int old_effective = effective_voffset();

		if (voffset == -1) {
			voffset = effective_voffset() + delta;
		} else if (voffset + delta < 0) {
			voffset = 0;
		} else {
			voffset += delta;
		}

		voffset = std::max(voffset, 0);
		int total = total_rows();
		if (pos.height < total)
			voffset = std::min(voffset, total - pos.height);

		const int new_effective = effective_voffset();
		const int diff = old_effective - new_effective;

		set_margins();
		in_margins = true;

		term->vscroll(diff);

		// If new < old, we need to render newly exposed lines at the top. If old < new, we render at the bottom.
		if (new_effective < old_effective) {
			term->jump(0, 0);
			for (int i = 0; i < diff; ++i) {
				*term << text_at_row(i);
				if (i < pos.height - 1)
					*term << "\n";
			}
		} else if (old_effective < new_effective) {
			term->jump(0, pos.height + diff);
			for (int i = pos.height + diff; i < pos.height; ++i) {
				*term << text_at_row(i);
				if (i < pos.height - 1)
					*term << "\n";
			}
		}

		reset_margins();
		in_margins = false;
	}

	int textbox::get_voffset() const {
		return voffset;
	}

	int textbox::effective_voffset() const {
		int total = total_rows();

		if (total <= pos.height)
			return 0;

		if (voffset == -1)
			return total <= pos.height? 0 : total - pos.height;

		return voffset;
	}

	void textbox::set_voffset(int new_voffset) {
		voffset = new_voffset;
	}

	int textbox::line_rows(const textline &line) const {
		// TODO: support doublewide characters.

		if (!wrap)
			return 1;
		
		return line.num_rows(pos.width);
	}

	int textbox::total_rows() const {
		if (!wrap)
			return lines.size();

		int rows = 0;
		for (const line_ptr &line: lines)
			rows += line_rows(*line);

		return rows;
	}

	void textbox::draw() {
		if (!can_draw())
			return;

		colored::draw();

		auto lock = term->lock_render();

		// It's assumed that the terminal is already in cbreak mode. If it's not, DECSLRM won't work and the left and
		// right margins won't be set.
		set_margins();
		in_margins = true;
		clear();
		jump();

		int effective = effective_voffset();

		if (0 <= effective && total_rows() <= effective) {
			// There's no need to draw anything if the box has been scrolled down beyond all its contents.
		} else {
			try {
				for (int i = 0; i < pos.height; ++i) {
					*term << text_at_row(i);
					if (i != pos.height - 1)
						*term << "\n";
				}
			} catch (std::out_of_range &err) {}
		}
		
		reset_margins();
		in_margins = false;
	}

	bool textbox::on_key(const key &k) {
		if (k == ktype::up_arrow) {
			vscroll(-1);
			draw();
		} else if (k == ktype::down_arrow) {
			vscroll(1);
			draw();
		} else if (k == ktype::left_arrow) {
			set_voffset(-1);
			draw();
		} else {
			return false;
		}

		return true;
	}

	bool textbox::can_draw() const {
		return parent != nullptr && term != nullptr && !term->suppress_output;
	}

	textbox & textbox::operator+=(const std::string &text) {
		if (!text.empty() && text.back() == '\n')
			lines.pop_back();

		std::unique_ptr<simpleline> ptr = std::make_unique<simpleline>(text, 0);
		lines.push_back(std::move(ptr));
		draw_new_line(*lines.back(), true);
		return *this;
	}

	textbox::operator std::string() const {
		std::string out = "";
		for (const std::unique_ptr<textline> &line: lines) {
			if (!out.empty())
				out += "\n";
			out += std::string(*line);
		}

		return out;
	}

	void swap(textbox &left, textbox &right) {
		swap(static_cast<control &>(left), static_cast<control &>(right));
		swap(static_cast<colored &>(left), static_cast<colored &>(right));
		std::swap(left.lines,      right.lines);
		std::swap(left.voffset,	   right.voffset);
		std::swap(left.wrap,       right.wrap);
		std::swap(left.in_margins, right.in_margins);
	}
}
