#include "ui/textbox.h"
#include "formicine/ansi.h"

namespace haunted::ui {
	textbox::textbox(container *parent, position pos, const std::vector<std::string> &contents, int offset):
	control(parent, pos), offset(offset) {
		parent->add_child(this);
		set_lines(contents);
	}

	textbox::textbox(container *parent, const std::vector<std::string> &contents, int offset): control(parent), offset(offset) {
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

		term->margins();
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



		term->margins();
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
