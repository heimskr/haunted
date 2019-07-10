#include "ui/textbox.h"

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


// Public instance methods


	textbox & textbox::operator+=(const std::string &line) {
		lines.push_back({line, 0});
		return *this;
	}

	textbox & textbox::operator+=(const textline &line) {
		lines.push_back(line);
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
