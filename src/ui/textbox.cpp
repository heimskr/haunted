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

		reset_margins();
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


// Public instance methods


	void textbox::clear() {
		lines.clear();
		draw();
	}

	void textbox::draw() {
		// It's assumed that the terminal is already in cbreak mode. If it's not, DECSLRM won't work and the left and
		// right margins won't be set.
		set_margins();

		reset_margins();
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
