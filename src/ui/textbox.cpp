#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "ui/textbox.h"
#include "formicine/ansi.h"
#include "tests/test.h"

namespace haunted::ui {
	std::string textline::text_at_row(size_t width, int row) const {
		if (row == 0) {
			return text.length() < width? text.substr(0, width) + std::string(width - text.length(), ' ')
				: text.substr(0, width);
		}

		size_t index = continuation + row * (width - continuation);
		if (index >= text.length())
			return std::string(width, ' ');

		std::string chunk = std::string(continuation, ' ') + text.substr(index, width - continuation);
		if (chunk.length() < width)
			return chunk + std::string(width - chunk.length(), ' ');

		return chunk;
	}

	textline::operator std::string() const {
		return text;
	}

	bool textline::operator==(const textline &other) const {
		return continuation == other.continuation && text == other.text;
	}

	std::ostream & operator<<(std::ostream &os, const textline &line) {
		os << line.text;
		return os;
	}

	textbox::textbox(container *parent, position pos, const std::vector<std::string> &contents): control(parent, pos) {
		if (parent != nullptr)
			parent->add_child(this);
		set_lines(contents);
		// add_child could modify the position, so we set the position again at the end to overoverwrite it.
		this->pos = pos;
	}

	textbox::textbox(container *parent, const std::vector<std::string> &contents): control(parent) {
		if (parent != nullptr)
			parent->add_child(this);
		set_lines(contents);
	}


// Private instance methods


	void textbox::set_lines(const std::vector<std::string> &strings) {
		lines.clear();
		for (const std::string &str: strings) {
			lines.push_back({str, 0});
		}
	}

	void textbox::draw_new_line(const textline &line) {
		if (!can_draw())
			return;


		// We need to subtract one to account for the fact that the new line is already in the buffer.
		int next = next_row() - 1;
		if (voffset != -1 && next < 0)
			return;

		set_margins();
		in_margins = true;

		int new_lines = line_rows(line);
		if (voffset == -1 && pos.height < total_rows()) {
			DBG("vscroll([" << -new_lines << "] -new_lines)");
			term->vscroll(-new_lines);
		}

		term->jump(0, next);
		int height = pos.height;
		for (int row = next, i = 0; row < height && i < new_lines; ++row, ++i) {
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
		if (pos.height < total - offset)
			return -1;

		return total - offset;
	}

	int textbox::line_rows(const textline &line) const {
		// TODO: support doublewide characters.
		if (!wrap)
			return 1;
		
		int length = ansi::strip(line.text).length();
		const int width = pos.width;

		if (length <= width)
			return 1;

		// Ignore all the text on the first line because it's not affected by continuation.
		length -= width;

		return length / (width - line.continuation) + (length % (width - line.continuation)? 2 : 1);
	}

	int textbox::total_rows() const {
		if (!wrap)
			return lines.size();

		int rows = 0;
		for (const textline &line: lines)
			rows += line_rows(line);
		return rows;
	}

	std::pair<textline &, int> textbox::line_at_row(int row) {
		if (lines.empty() || row >= total_rows())
			throw std::out_of_range("Invalid row index: " + std::to_string(row));

		if (!wrap)
			return {lines[row], 0};

		int line_count = lines.size(), index = 0, row_count = 0, last_count = 0, offset = -1;

		for (index = 0, row_count = 0;; ++index) {
			last_count = line_rows(lines[index]);
			if (row_count <= row && row < row_count + last_count) {
				offset = row - row_count;
				break;
			} else {
				row_count += last_count;
			}
		}
		
		if (line_count <= index)
			throw std::out_of_range("Line index too large: " + std::to_string(index));

 		return {lines[index], offset == -1? row - row_count : offset};
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
		
		textline line;
		size_t offset;

		if (pos.height <= row || row < 0)
			return "";

		try {
			std::tie(line, offset) = line_at_row(row + effective_voffset());
		} catch (std::out_of_range &) {
			return std::string(cols, ' ');
		}

		if (offset == 0)
			return line.text.length() <= cols? line.text + std::string(cols - line.text.length(), ' ')
				: line.text.substr(0, cols);

		// Number of chars visible per row on a continued line
		size_t continuation_chars = cols - line.continuation;

		// Ignore the first line. 
		std::string str = line.text.substr(cols);

		// Erase the continued lines after the first line and before the continuation at the given row.
		str.erase(0, (offset - 1) * continuation_chars);

		// Erase all characters after the continuation if any remain.
		if (continuation_chars < str.length())
			str.erase(continuation_chars, std::string::npos);

		// Return the continuation padding plus the segment of text visible on the row.
		if (line.continuation + str.length() < cols)
			return std::string(line.continuation, ' ') + str + std::string(cols - line.continuation - str.size(), ' ');
		return std::string(line.continuation, ' ') + str;
	}


// Public instance methods


	void textbox::clear_lines() {
		lines.clear();
		if (0 < voffset)
			voffset = 0;
		draw();
	}

	void textbox::vscroll(int delta) {
		// TODO: scroll the terminal and fill in the empty lines.

		if (voffset == -1) {
			voffset = effective_voffset() + delta;
		} else if (voffset + delta < 0) {
			voffset = 0;
		} else {
			voffset += delta;
		}

		voffset = std::max(voffset, 0);
		int total = total_rows();
		if (pos.height < total) {
			voffset = std::min(voffset, total - pos.height);
		}
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

	void textbox::draw() {
		if (!can_draw())
			return;

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

	textbox & textbox::operator+=(const std::string &line) {
		if (!line.empty() && line.back() == '\n')
			lines.pop_back();

		lines.push_back({line, 0});
		draw_new_line(lines.back());
		return *this;
	}

	textbox & textbox::operator+=(const textline &line) {
		lines.push_back(line);
		draw_new_line(line);
		return *this;
	}

	textbox::operator std::string() const {
		std::string out = "";
		for (const textline &line: lines) {
			if (!out.empty())
				out += "\n";
			out += line.text;
		}

		return out;
	}
}
