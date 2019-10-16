#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "haunted/ui/textbox.h"

#include "lib/formicine/performance.h"

namespace haunted::ui {
	bool textline::operator==(const textline &other) const {
		return continuation == other.continuation && std::string(*this) == std::string(other);
	}

	simpleline::simpleline(const std::string &text_, int continuation_): textline(continuation_), text(text_) {
		text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
		text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
	}

	std::string textline::text_at_row(size_t width, int row, bool pad_right) const {
		auto w = formicine::perf.watch("textline::text_at_row");
		const std::string text = std::string(*this);
		const size_t text_length = ansi::length(text);

		if (row == 0) {
			return text_length < width? ansi::substr(text, 0, width) + std::string(width - text_length, ' ')
			     : ansi::substr(text, 0, width);
		}

		const size_t index = continuation + row * (width - continuation);
		if (index >= text_length)
			return pad_right? std::string(width, ' ') : "";

		std::string chunk = std::string(continuation, ' ') + ansi::substr(text, index, width - continuation);
		const size_t chunk_length = ansi::length(chunk);
		if (pad_right && chunk_length < width)
			return chunk + std::string(width - chunk_length, ' ');

		return chunk;
	}

	int textline::num_rows(int width) const {
		const std::string text = ansi::strip(*this);
		// auto w = formicine::perf.watch("textline::num_rows");

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
		auto w = formicine::perf.watch("operator<<(std::ostream, simpleline)");
		os << line.text;
		return os;
	}

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
	}

	void textbox::draw_new_line(const textline &line, bool inserted) {
		if (!can_draw())
			return;

		auto lock = term->lock_render();
		auto w = formicine::perf.watch("textbox::draw_new_line");

		const int new_lines = line_rows(line);
		const int offset = inserted? new_lines : 0;

		int next = next_row(offset);
		if (!autoscroll && next < 0)
			return;

		try_margins([&]() {
			apply_colors();

			int upscroll = 0;

			if (autoscroll && pos.height < total_rows() - voffset) {
				const int blanks_at_bottom = pos.height - (inserted? total_rows() - new_lines : total_rows()) + voffset;
				upscroll = inserted? new_lines - blanks_at_bottom : blanks_at_bottom;
				term->vscroll(-upscroll);

				// After we scroll the terminal, there's some new space for lines to be in, whereas there was no space
				// before. Because of that, we have to recalculate the next row by using the number of new lines to
				// decrease the vertical offset the next_row method uses.
				next = next_row(new_lines) - offset + blanks_at_bottom;
			}

			// Here's a kludge!
			if (next < 0) {
				next = pos.height - offset;
			}

			term->jump(0, next);
			for (int row = next, i = 0; row < pos.height && i < new_lines; ++row, ++i) {
				if (i > 0)
					*term << "\n";
				*term << line.text_at_row(pos.width, i, true);
			}

			voffset += upscroll;
			uncolor();
		});

		term->jump_to_focused();
	}

	int textbox::next_row(int offset_offset) const {
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
		const int continuation = line->continuation;

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
		if (pad_right && continuation + str_length < cols)
			return std::string(continuation, ' ') + str + std::string(cols - continuation - str_length, ' ');
		return std::string(continuation, ' ') + str;
	}

	bool textbox::do_scroll(size_t rows) {
		if (pos.height < total_rows() - voffset) {
			vscroll(rows);
			return true;
		}

		return false;
	}


// Public instance methods


	void textbox::clear_lines() {
		lines.clear();
		if (0 < voffset)
			voffset = 0;
		draw();
	}

	void textbox::vscroll(int delta) {
		auto w = formicine::perf.watch("textbox::vscroll");

		const int old_voffset = voffset;
		voffset = std::max(voffset + delta, 0);

		// Don't let the voffset extend past the point where the last line of text is just above the first row.
		const int total = total_rows();
		if (pos.height < total)
			voffset = std::min(voffset, total);

		if (!can_draw())
			return;

		auto lock = term->lock_render();
		const int diff = old_voffset - voffset;

		try_margins([&]() {
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

	int textbox::line_rows(const textline &line) const {
		// TODO: support doublewide characters.

		auto w = formicine::perf.watch("textbox::line_rows");

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

		auto w = formicine::perf.watch("textbox::draw");
		auto lock = term->lock_render();

		try_margins([&]() {
			term->hide();
			apply_colors();
			clear_rect();

			if (0 <= voffset && total_rows() <= voffset) {
				// There's no need to draw anything if the box has been scrolled down beyond all its contents.
			} else {
				try {
					std::string text {};
					for (int i = 0; i < pos.height; ++i) {
						if (i != 0)
							text.push_back('\n');
						text += text_at_row(i, false);
					}

					apply_colors();
					term->jump(0, 0);
					*term << text;
				} catch (std::out_of_range &err) {}
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
		mouse_report relative = report;
		relative.x -= pos.left;
		relative.y -= pos.top;
		try {
			textline *line;
			std::tie(line, relative.y) = line_at_row(relative.y + voffset);
			if (line) {
				line->on_click(relative);
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
		const size_t num_rows = ptr->num_rows(pos.width);
		lines.push_back(std::move(ptr));
		if (!do_scroll(num_rows))
			draw_new_line(*lines.back(), true);
		return *this;
	}

	textbox::operator std::string() const {
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
		std::swap(left.wrap,       right.wrap);
	}
}
