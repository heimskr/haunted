#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "haunted/ui/textbox.h"

#include "lib/formicine/performance.h"

namespace haunted::ui {


// Public constructors


	textbox::textbox(container *parent_, position pos_, const std::vector<std::string> &contents):
	colored_control(parent_, pos_) {
		if (parent_ != nullptr)
			parent_->add_child(this);
		set_lines(contents);
		// add_child could modify the position, so we set the position again at the end to overoverwrite it.
		pos = pos_;
	}

	textbox::textbox(container *parent_, const std::vector<std::string> &contents): colored_control(parent_) {
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

		rows_dirty();
	}

	void textbox::draw_new_line(textline &line, bool inserted) {
		if (!can_draw())
			return;

		auto lock = term->lock_render();
		auto w = formicine::perf.watch("textbox::draw_new_line");

		const int new_lines = line_rows(line);
		const int offset = inserted? new_lines : 0;

		int next = next_row(offset);
		if (!autoscroll && next < 0)
			return;

		try_margins([&, this]() {
			apply_colors();

			// It's assumed that whatever's calling this method will deal with autoscroll on its own.

			// If next < 0, it's because autoscrolling didn't scroll to make space for the line, which means it's below
			// the visible area. (Maybe above if scrolling up doesn't have a boundary?) Because it's out of sight,
			// there's no need to print anything. Doing so would overwrite the bottom line with incorrect text.
			if (next < 0)
				return;

			term->jump(0, next);
			for (int row = next, i = 0; row < pos.height && i < new_lines; ++row, ++i) {
				if (i > 0)
					*term << "\n";
				*term << line.text_at_row(pos.width, i, true);
			}

			uncolor();
		});

		term->jump_to_focused();
	}

	int textbox::next_row(int offset_offset) {
		int offset = voffset + offset_offset;
		int total = total_rows();

		// Return -1 if the next row is below the visible area.
		if (pos.height <= total - offset)
			return -1;

		return total - offset;
	}

	std::pair<textline *, int> textbox::line_at_row(int row) {
		if (lines.empty() || row >= total_rows())
			throw std::out_of_range("Invalid row index: " + std::to_string(row));

		auto w = formicine::perf.watch("textbox::line_at_row");

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

	std::string textbox::text_at_row(int row, bool pad_right) {
		const size_t cols = pos.width;
		auto w = formicine::perf.watch("textbox::text_at_row");
		// w.canceled = true;
		textline *line;
		size_t offset;

		if (pos.height <= row || row < 0) {
			return "";
		}

		if (lines.empty() || (row + voffset) >= total_rows()) {
			return pad_right? std::string(cols, ' ') : "";
		} else {
			std::tie(line, offset) = line_at_row(row + voffset);
		}

		const std::string line_text = std::string(*line);
		const int continuation = line->get_continuation();

		if (offset == 0) {
			const size_t line_length = ansi::length(line_text);
			if (pad_right) {
				return line_length <= cols? line_text + std::string(cols - line_length, ' ')
					: ansi::substr(line_text, 0, cols);
			} else {
				return line_length <= cols? line_text : ansi::substr(line_text, 0, cols);
			}
		}

		// Number of chars visible per row on a continued line
		size_t continuation_chars = cols - continuation;

		// Ignore the first line. 
		std::string str = ansi::substr(line_text, cols);

		// Erase the continued lines after the first line and before the continuation at the given row.
		ansi::erase(str, 0, (offset - 1) * continuation_chars);

		// Erase all characters after the continuation if any remain.
		if (continuation_chars < ansi::length(str))
			ansi::erase(str, continuation_chars, std::string::npos);

		const size_t str_length = ansi::length(str);
		// Return the continuation padding plus the segment of text visible on the row.
		if (pad_right && continuation + str_length < cols)
			return std::string(continuation, ' ') + str + std::string(cols - continuation - str_length, ' ');
		return std::string(continuation, ' ') + str;
	}

	bool textbox::do_scroll(size_t rows) {
		if (autoscroll && pos.height == total_rows() - voffset) {
			vscroll(rows);
			return true;
		}

		return false;
	}

	void textbox::rows_dirty() {
		total_rows_ = -1;
	}


// Public instance methods


	void textbox::clear_lines() {
		lines.clear();
		rows_dirty();
		if (0 < voffset)
			voffset = 0;
		draw();
	}

	void textbox::vscroll(int delta) {
		auto w = formicine::perf.watch("textbox::vscroll");

		const int total = total_rows();
		const int old_voffset = voffset;

		voffset = std::max(std::min(total - static_cast<int>(scroll_buffer), voffset + delta), 0);

		// Don't let the voffset extend past the point where the (scroll_buffer + 1)th-last line of text is just above
		// the first row.
		if (pos.height < total)
			voffset = std::min(voffset, total - static_cast<int>(scroll_buffer));

		if (!can_draw())
			return;

		auto lock = term->lock_render();
		const int diff = old_voffset - voffset;

		try_margins([&, this]() {
			apply_colors();
			term->vscroll(diff);

			// If new < old, we need to render newly exposed lines at the top. If old < new, we render at the bottom.
			if (voffset < old_voffset) {
				term->jump(0, 0);
				for (int i = 0; i < diff; ++i) {
					*term << text_at_row(i);
					if (i < pos.height - 1)
						*term << "\n";
				}
			} else if (old_voffset < voffset) {
				term->jump(0, pos.height + diff);
				for (int i = pos.height + diff; i < pos.height; ++i) {
					*term << text_at_row(i);
					if (i < pos.height - 1)
						*term << "\n";
				}
			}

			uncolor();
		});

		term->jump_to_focused();
	}

	int textbox::get_voffset() const {
		return voffset;
	}

	void textbox::set_voffset(int new_voffset) {
		const int old_voffset = voffset;
		if (new_voffset != old_voffset) {
			voffset = old_voffset;
			vscroll(new_voffset - old_voffset);
		}
	}

	void textbox::set_autoscroll(bool autoscroll_) {
		if (autoscroll != autoscroll_)
			autoscroll = autoscroll_;
	}

	int textbox::line_rows(textline &line) {
		// TODO: support doublewide characters.

		auto w = formicine::perf.watch("textbox::line_rows");

		return line.num_rows(pos.width);
	}

	int textbox::total_rows() {
		auto w = formicine::perf.watch("textbox::total_rows");

		if (total_rows_ != -1) {
			return total_rows_;
		}


		total_rows_ = 0;
		for (const line_ptr &line: lines)
			total_rows_ += line_rows(*line);

		return total_rows_;
	}

	void textbox::draw() {
		if (!can_draw())
			return;

		auto w = formicine::perf.watch("textbox::draw");
		auto lock = term->lock_render();

		try_margins([&, this]() {
			term->hide();
			apply_colors();
			clear_rect();

			if (0 <= voffset && total_rows() <= voffset) {
				// There's no need to draw anything if the box has been scrolled down beyond all its contents.
			} else {
				try {
					std::string text {};
					text.reserve(pos.height * pos.width);
					for (int i = 0; i < pos.height; ++i) {
						if (i != 0)
							text.push_back('\n');
						text += text_at_row(i, false);
					}

					term->jump(0, 0);
					*term << text;
					apply_colors();
				} catch (const std::out_of_range &) {}
			}
			
			uncolor();
			term->show();
		});

		term->jump_to_focused();
	}

	bool textbox::on_key(const key &k) {
		return key_fn? key_fn(k) : default_on_key(k);
	}

	bool textbox::on_mouse(const mouse_report &report) {
		if (report.action == mouse_action::scrollup) {
			vscroll(-1);
			return true;
		} else if (report.action == mouse_action::scrolldown) {
			vscroll(1);
			return true;
		}

		mouse_report relative = report;
		relative.x -= pos.left;
		relative.y -= pos.top;

		try {
			textline *line;
			std::tie(line, relative.y) = line_at_row(relative.y + voffset);
			if (line) {
				line->on_mouse(relative);
				return true;
			}
		} catch(const std::out_of_range &) {}

		return false;
	}

	bool textbox::default_on_key(const key &k) {
		if (k == ktype::up_arrow) {
			vscroll(-1);
			draw();
		} else if (k == ktype::down_arrow) {
			vscroll(1);
			draw();
		} else if (k == ktype::left_arrow) {
			set_autoscroll(true);
			draw();
		} else {
			return false;
		}

		return true;
	}

	bool textbox::can_draw() const {
		return parent != nullptr && term != nullptr && !term->suppress_output;
	}

	void textbox::focus() {
		control::focus();
		colored::focus();
	}

	textbox & textbox::operator+=(const std::string &text) {
		auto w = formicine::perf.watch("textbox::operator+=");
		if (!text.empty() && text.back() == '\n')
			lines.pop_back();

		std::unique_ptr<simpleline> ptr = std::make_unique<simpleline>(text, 0);
		const size_t nrows = ptr->num_rows(pos.width);
		lines.push_back(std::move(ptr));
		if (!do_scroll(nrows))
			draw_new_line(*lines.back(), true);
		rows_dirty();
		return *this;
	}

	textbox::operator std::string() {
		auto w = formicine::perf.watch("textbox::operator std::string");
		std::string out = "";
		for (const line_ptr &line: lines) {
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
	}
}
