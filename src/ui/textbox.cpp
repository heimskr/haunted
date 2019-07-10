#include "ui/textbox.h"
#include "formicine/ansi.h"

namespace haunted::ui {
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
		if (!wrap)
			return {lines[row], 0};

		size_t index, row_count, last_count;
		for (index = 0, row_count = 0; row_count < row; ++index)
			row_count += last_count = line_rows(lines[index]);
		
		return {lines[index], row - row_count - last_count};
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


// Public instance methods


	void textbox::clear_lines() {
		lines.clear();
		draw();
	}

	void textbox::draw() {
		// It's assumed that the terminal is already in cbreak mode. If it's not, DECSLRM won't work and the left and
		// right margins won't be set.
		set_margins();
		in_margins = true;
		clear();

		if (0 <= voffset && total_rows() <= static_cast<size_t>(voffset)) {
			// There's no need to draw anything if the box has been scrolled down beyond all its contents.
		} else {

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
