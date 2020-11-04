#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "haunted/ui/Textbox.h"

#include "lib/formicine/performance.h"

namespace Haunted::UI {


// Public constructors


	Textbox::Textbox(Container *parent_, const Position &pos_, const std::vector<std::string> &contents):
	ColoredControl(parent_, pos_) {
		if (parent_)
			parent_->addChild(this);
		setLines(contents);
		// addChild could modify the position, so we set the position again at the end to overoverwrite it.
		position = pos_;
	}

	Textbox::Textbox(Container *parent_, const std::vector<std::string> &contents): ColoredControl(parent_) {
		if (parent_)
			parent_->addChild(this);
		setLines(contents);
	}


// Private instance methods


	void Textbox::setLines(const std::vector<std::string> &strings) {
		lines.clear();
		for (const std::string &str: strings) {
			std::shared_ptr<SimpleLine> ptr = std::make_shared<SimpleLine>(str, 0);
			lines.push_back(std::move(ptr));
		}

		rowsDirty();
	}

	void Textbox::drawNewLine(TextLine &line, bool inserted) {
		if (!canDraw())
			return;

		auto lock = terminal->lockRender();
		auto w = formicine::perf.watch("Textbox::drawNewLine");

		const int new_lines = lineRows(line);
		const int offset = inserted? new_lines : 0;

		int next = nextRow(offset);
		if (!autoscroll && next < 0)
			return;

		tryMargins([&, this]() {
			applyColors();

			// It's assumed that whatever's calling this method will deal with autoscroll on its own.

			// If next < 0, it's because autoscrolling didn't scroll to make space for the line, which means it's below
			// the visible area. (Maybe above if scrolling up doesn't have a boundary?) Because it's out of sight,
			// there's no need to print anything. Doing so would overwrite the bottom line with incorrect text.
			if (next < 0)
				return;

			terminal->jump(0, next);
			for (int row = next, i = 0; row < position.height && i < new_lines; ++row, ++i) {
				if (i > 0)
					*terminal << "\n";
				*terminal << line.textAtRow(position.width, i, true);
			}

			uncolor();
		});

		terminal->jumpToFocused();
	}

	int Textbox::nextRow(int offset_offset) {
		int offset = voffset + offset_offset;
		int total = totalRows();

		// Return -1 if the next row is below the visible area.
		if (position.height <= total - offset)
			return -1;

		return total - offset;
	}

	std::pair<TextLine *, int> Textbox::lineAtRow(int row) {
		if (lines.empty() || row >= totalRows())
			throw std::out_of_range("Invalid row index: " + std::to_string(row));

		auto w = formicine::perf.watch("Textbox::lineAtRow");

		int line_count = lines.size(), index = 0, row_count = 0, last_count = 0, offset = -1;

		auto iter = lines.begin();
		for (index = 0, row_count = 0; index < line_count; ++index) {
			last_count = lineRows(**(iter++));
			if (row_count <= row && row < row_count + last_count) {
				offset = row - row_count;
				break;
			} else {
				row_count += last_count;
			}
		}

		if (line_count <= index)
			throw std::out_of_range("Line index too large: " + std::to_string(index));

		return {std::next(lines.begin(), index)->get(), offset == -1? row - row_count : offset};
	}

	std::string Textbox::textAtRow(int row, bool pad_right) {
		const size_t cols = position.width;
		auto w = formicine::perf.watch("Textbox::textAtRow");
		// w.canceled = true;
		TextLine *line;
		size_t offset;

		if (position.height <= row || row < 0)
			return "";

		if (lines.empty() || (row + voffset) >= totalRows()) {
			return pad_right? std::string(cols, ' ') : "";
		} else {
			std::tie(line, offset) = lineAtRow(row + voffset);
		}

		const std::string line_text = std::string(*line);
		const int continuation = line->getContinuation();

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

	bool Textbox::doScroll(size_t rows) {
		if (autoscroll && position.height == totalRows() - voffset) {
			vscroll(rows);
			return true;
		}

		return false;
	}

	void Textbox::rowsDirty() {
		totalRows_ = -1;
	}

	void Textbox::linesDirty() {
		for (LinePtr line: lines)
			line->markDirty();
	}

	void Textbox::markDirty() {
		rowsDirty();
		linesDirty();
	}


// Public instance methods


	void Textbox::clearLines() {
		lines.clear();
		rowsDirty();
		if (0 < voffset)
			voffset = 0;
		draw();
	}

	void Textbox::vscroll(int delta) {
		auto w = formicine::perf.watch("Textbox::vscroll");

		const int total = totalRows();
		const int old_voffset = voffset;

		voffset = std::max(std::min(total - static_cast<int>(scroll_buffer), voffset + delta), 0);

		// Don't let the voffset extend past the point where the (scroll_buffer + 1)th-last line of text is just above
		// the first row.
		if (position.height < total)
			voffset = std::min(voffset, total - static_cast<int>(scroll_buffer));

		if (!canDraw())
			return;

		auto lock = terminal->lockRender();
		const int diff = old_voffset - voffset;

		tryMargins([&, this]() {
			applyColors();
			terminal->vscroll(diff);

			// If new < old, we need to render newly exposed lines at the top. If old < new, we render at the bottom.
			if (voffset < old_voffset) {
				terminal->jump(0, 0);
				for (int i = 0; i < diff; ++i) {
					*terminal << textAtRow(i);
					if (i < position.height - 1)
						*terminal << "\n";
				}
			} else if (old_voffset < voffset) {
				terminal->jump(0, position.height + diff);
				for (int i = position.height + diff; i < position.height; ++i) {
					*terminal << textAtRow(i);
					if (i < position.height - 1)
						*terminal << "\n";
				}
			}

			uncolor();
		});

		terminal->jumpToFocused();
	}

	int Textbox::getVoffset() const {
		return voffset;
	}

	void Textbox::setVoffset(int new_voffset) {
		const int old_voffset = voffset;
		if (new_voffset != old_voffset) {
			voffset = old_voffset;
			vscroll(new_voffset - old_voffset);
		}
	}

	void Textbox::setAutoscroll(bool autoscroll_) {
		if (autoscroll != autoscroll_)
			autoscroll = autoscroll_;
	}

	int Textbox::lineRows(TextLine &line) {
		// TODO: support doublewide characters.

		auto w = formicine::perf.watch("textbox::lineRows");
		auto lock = lockLines();
		return line.numRows(position.width);
	}

	int Textbox::totalRows() {
		auto w = formicine::perf.watch("textbox::total_rows");
		auto lock = lockLines();

		if (totalRows_ != -1)
			return totalRows_;

		totalRows_ = 0;
		for (const LinePtr &line: lines)
			totalRows_ += lineRows(*line);

		return totalRows_;
	}

	void Textbox::draw() {
		if (!canDraw())
			return;

		auto w = formicine::perf.watch("textbox::draw");
		auto lock = terminal->lockRender();
		auto line_lock = lockLines();

		tryMargins([&, this]() {
			terminal->hide();
			clearRect();
			applyColors();

			if (0 <= voffset && totalRows() <= voffset) {
				// There's no need to draw anything if the box has been scrolled down beyond all its contents.
			} else {
				try {
					std::string text {};
					text.reserve(position.height * position.width);
					for (int i = 0; i < position.height; ++i) {
						if (i != 0)
							text.push_back('\n');
						text += textAtRow(i, false);
					}

					terminal->jump(0, 0);
					*terminal << text;
					applyColors();
				} catch (const std::out_of_range &) {}
			}
			
			uncolor();
			terminal->show();
		});

		terminal->jumpToFocused();
	}

	void Textbox::resize(const Haunted::Position &new_pos) {
		ColoredControl::resize(new_pos);
		markDirty();
	}

	bool Textbox::onKey(const Key &key) {
		return keyFunction? keyFunction(key) : defaultOnKey(key);
	}

	bool Textbox::onMouse(const MouseReport &report) {
		if (report.action == MouseAction::ScrollUp) {
			vscroll(-1);
			return true;
		} else if (report.action == MouseAction::ScrollDown) {
			vscroll(1);
			return true;
		}

		MouseReport relative = report;
		relative.x -= position.left;
		relative.y -= position.top;

		try {
			TextLine *line;
			std::tie(line, relative.y) = lineAtRow(relative.y + voffset);
			if (line) {
				line->onMouse(relative);
				return true;
			}
		} catch(const std::out_of_range &) {}

		return false;
	}

	bool Textbox::defaultOnKey(const Key &k) {
		if (k == KeyType::UpArrow) {
			vscroll(-1);
			draw();
		} else if (k == KeyType::DownArrow) {
			vscroll(1);
			draw();
		} else if (k == KeyType::LeftArrow) {
			setAutoscroll(true);
			draw();
		} else {
			return false;
		}

		return true;
	}

	bool Textbox::canDraw() const {
		return parent != nullptr && terminal != nullptr && !terminal->suppressOutput && !suppressDraw;
	}

	void Textbox::focus() {
		Control::focus();
		Colored::focus();
	}

	void Textbox::redrawLine(TextLine &to_redraw) {
		int rows = 0;
		auto lock = lockLines();
		for (LinePtr &line: lines) {
			if (line.get() == &to_redraw)
				break;
			rows += lineRows(*line);
		}

		int next = rows - voffset;
		if (voffset <= rows && next < position.height) {
			// The line is in view.
			to_redraw.markDirty();
			to_redraw.clean(position.width);
			const int new_lines = lineRows(to_redraw);
			tryMargins([&, this]() {
				applyColors();

				terminal->jump(0, next);
				for (int row = next, i = 0; row < position.height && i < new_lines; ++row, ++i) {
					if (i > 0)
						*terminal << "\n";
					*terminal << to_redraw.textAtRow(position.width, i, true);
				}

				uncolor();
			});
		}
	}

	Textbox & Textbox::operator+=(const std::string &text) {
		auto w = formicine::perf.watch("textbox::operator+=");
		auto lock = lockLines();
		if (!text.empty() && text.back() == '\n')
			return *this += text.substr(0, text.size() - 1);

		std::shared_ptr<SimpleLine> ptr = std::make_shared<SimpleLine>(text, 0);
		const size_t nrows = ptr->numRows(position.width);
		doScroll(nrows);
		lines.push_back(std::move(ptr));
		rowsDirty();
		drawNewLine(*lines.back(), true);
		return *this;
	}

	Textbox::operator std::string() {
		auto w = formicine::perf.watch("textbox::operator std::string");
		auto lock = lockLines();
		std::string out = "";
		for (const LinePtr &line: lines) {
			if (!out.empty())
				out += "\n";
			out += std::string(*line);
		}

		return out;
	}

	void swap(Haunted::UI::Textbox &left, Haunted::UI::Textbox &right) {
		swap(static_cast<Haunted::UI::Control &>(left), static_cast<Haunted::UI::Control &>(right));
		swap(static_cast<Haunted::UI::Colored &>(left), static_cast<Haunted::UI::Colored &>(right));
		std::swap(left.lines,   right.lines);
		std::swap(left.voffset, right.voffset);
	}
}
