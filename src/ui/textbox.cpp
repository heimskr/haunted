#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "ui/textbox.h"
#include "../../lib/formicine/ansi.h"

namespace haunted::ui {
	textline::operator std::string() const {
		return text;
	}

	std::ostream & operator<<(std::ostream &os, const textline &line) {
		os << line.text;
		return os;
	}

	textbox::textbox(container *parent, position pos, const std::vector<std::string> &contents): control(parent, pos) {
		parent->add_child(this);
		set_lines(contents);
	}

	textbox::textbox(container *parent, const std::vector<std::string> &contents): control(parent) {
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

	void textbox::draw_new_line(const textline &) {
		set_margins();
		in_margins = true;

		// TODO: implement

		reset_margins();
		in_margins = false;
	}

	size_t textbox::line_rows(const textline &line) const {
		// TODO: support doublewide characters.
		if (!wrap)
			return 1;
		
		size_t length = ansi::strip(line.text).length();
		const size_t width = pos.width;

		if (length <= width)
			return 1;

		length -= width; // Ignore all the text on the first line because it's not affected by continuation.
		return length / (width - line.continuation) + (length % (width - line.continuation)? 2 : 1);
	}

	size_t textbox::total_rows() const {
		if (!wrap)
			return lines.size();

		size_t out = 0;
		for (const textline &line: lines)
			out += line_rows(line);
		return out;
	}

	std::pair<textline &, size_t> textbox::line_at_row(size_t row) {
		if (lines.empty() || row >= lines.size())
			throw std::out_of_range("Invalid row index: " + std::to_string(row));

		if (!wrap)
			return {lines[row], 0};

		size_t line_count = lines.size();
		size_t index = 0, row_count = 0, last_count = 0;

		for (index = 0, row_count = 0; row_count < row; ++index) {
			if (line_count <= index)
				throw std::out_of_range("Line index too large: " + std::to_string(index));
			row_count += last_count = line_rows(lines[index]);
		}
		
		if (line_count <= index)
			throw std::out_of_range("Line index too large: " + std::to_string(index));
		
		return {lines[index], row - row_count};
	}

	void textbox::clear() {
		bool should_reset_margins = false;
		if (!in_margins) {
			set_margins();
			should_reset_margins = true;
		}

		for (int i = 0; i < pos.height; ++i) {
			if (i) ansi::down();
			ansi::delete_chars(pos.width);
		}

		if (should_reset_margins)
			reset_margins();
	}

	std::string textbox::text_at_row(size_t row) {
		size_t cols = term->get_cols();
		
		textline line;
		size_t offset;
		std::tie(line, offset) = line_at_row(row + effective_voffset());

		if (offset == 0)
			return line.text.length() <= cols? line.text : line.text.substr(0, cols);

		// Number of chars visible per row on a continued line
		size_t continuation_chars = cols - line.continuation;

		// Ignore the first line. 
		std::string str = line.text.substr(cols);

		// Erase the continued lines after the first line and before the continuation at the given row
		str.erase(0, (offset - 1) * continuation_chars);
		// and all characters after the continuation.
		str.erase(continuation_chars, std::string::npos);

		// Return the continuation padding plus the segment of text visible on the row.
		return std::string(line.continuation, ' ') + str;
	}


// Public instance methods


	void textbox::clear_lines() {
		lines.clear();
		draw();
	}

	void textbox::vscroll(int delta) {
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
		if (voffset == -1) {
			size_t total = total_rows();
			return static_cast<int>(total) <= pos.height? 0 : total - pos.height;
		}

		return voffset;
	}

	void textbox::set_voffset(int new_voffset) {
		voffset = new_voffset;
	}

	void textbox::draw() {
		// It's assumed that the terminal is already in cbreak mode. If it's not, DECSLRM won't work and the left and
		// right margins won't be set.
		set_margins();
		in_margins = true;
		clear();
		jump();

		int effective = effective_voffset();

		if (0 <= effective && total_rows() <= static_cast<size_t>(effective)) {
			// There's no need to draw anything if the box has been scrolled down beyond all its contents.
		} else {
			DBG("effective = " << effective << " (" << total_rows() << " - " << pos.height << ")");
			try {
				for (int i = 0; i < pos.height; ++i) {
					*term << text_at_row(i);
					if (i != pos.height -1) {
						*term << "\n";
					}
				}
			} catch (std::out_of_range &err) {}
		}
		
		reset_margins();
		in_margins = false;
	}

	textbox & textbox::operator+=(const std::string &line) {
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
