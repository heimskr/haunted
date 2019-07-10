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

	void textbox::set_lines(const std::vector<std::string> &strings) {
		lines.clear();
		for (const std::string &str: strings) {
			lines.push_back({str, 0});
		}
	}
}
