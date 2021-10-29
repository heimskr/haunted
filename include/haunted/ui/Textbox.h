#pragma once

#include <deque>
#include <functional>
#include <list>
#include <mutex>
#include <string>
#include <utility>

#include "haunted/ui/ColoredControl.h"
#include "haunted/core/Terminal.h"
#include "haunted/core/Util.h"

#include "haunted/ui/TextLine.h"
#include "haunted/ui/SimpleLine.h"

#include "lib/formicine/performance.h"

namespace Haunted::Tests {
	class maintest;
}

namespace Haunted::UI {
	/**
	 * Represents a multiline box of text.
	 */
	template <template <typename... T> typename C>
	class Textbox: public ColoredControl {
		friend class Haunted::Tests::maintest;

		public:
			using LinePtr = std::shared_ptr<TextLine<C>>;

		protected:
			/** Holds all the textlines in the box. */
			C<LinePtr> lines;

			/** The number of rows the container has been scrolled vertically. */
			ssize_t voffset = 0;

			/** Whether the textbox should automatically scroll to keep up with lines added to the bottom. */
			bool autoscroll = false;

			/** Used for locking when doing operations on lines. */
			std::recursive_mutex lineMutex;

			std::unique_lock<std::recursive_mutex> lockLines() { return std::unique_lock(lineMutex); }

			/** Empties the buffer and replaces it with 0-continuation lines from a vector of string. */
			void setLines(const std::vector<std::string> &strings) {
				lines.clear();
				for (const std::string &str: strings) {
					std::shared_ptr<SimpleLine<C>> ptr = std::make_shared<SimpleLine<C>>(str, 0);
					lines.push_back(std::move(ptr));
				}

				rowsDirty();
			}

			/** When a new line is added, it's usually not necessary to completely redraw the component. Instead,
			 *  scrolling the component and printing only the new line is sufficient.
			 *  @param inserted Whether the line has already been inserted into the textbox's collection. */
			void drawNewLine(TextLine<C> &line, bool inserted = false) {
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

					// If next < 0, it's because autoscrolling didn't scroll to make space for the line, which means
					// it's below the visible area. (Maybe above if scrolling up doesn't have a boundary?) Because it's
					// out of sight, there's no need to print anything. Doing so would overwrite the bottom line with
					// incorrect text.
					if (next < 0)
						return;

					terminal->jump(0, next);
					for (int row = next, i = 0; row < position.height && i < new_lines; ++row, ++i) {
						if (0 < i)
							*terminal << "\n";
						*terminal << line.textAtRow(position.width, i, true);
					}

					uncolor();
				});

				terminal->jumpToFocused();
			}

			/** Returns the row on which the next line should be drawn or -1 if it's out of bounds. */
			ssize_t nextRow(ssize_t offset_offset = 0) {
				ssize_t offset = voffset + offset_offset;
				size_t total = totalRows();

				// Return -1 if the next row is below the visible area.
				if (position.height <= ssize_t(total) - offset)
					return -1;

				return ssize_t(total) - offset;
			}

			/** Returns a pair of the line at a given row (ignoring voffset and zero-based) and the number of rows past
			 *  the start of the line. For example, if the textbox contains one line that occupies a single row and a
			 *  second line that spans 5 rows, then calling this function with 4 will return {lines[1], 3}. */
			std::pair<TextLine<C> *, size_t> lineAtRow(size_t row) {
				if (lines.empty() || row >= totalRows())
					throw std::out_of_range("Invalid row index: " + std::to_string(row));

				auto w = formicine::perf.watch("Textbox::lineAtRow");

				if (!allowWrap)
					return {lines.at(row).get(), row};

				size_t line_count = lines.size(), index = 0, row_count = 0, last_count = 0, offset = -1;

				auto iter = lines.begin();
				for (index = 0, row_count = 0; index < line_count; ++index) {
					last_count = lineRows(**(iter++));
					if (row_count <= row && row < row_count + last_count) {
						offset = row - row_count;
						break;
					} else
						row_count += last_count;
				}

				if (line_count <= index)
					throw std::out_of_range("Line index too large: " + std::to_string(index));

				return {std::next(lines.begin(), index)->get(), offset == -1ul? row - row_count : offset};
			}

			/** Returns the string to print on a given row (zero-based) of the textbox. Handles text wrapping and
			 *  scrolling automatically. */
			std::string textAtRow(size_t row, bool pad_right = true) {
				const ssize_t cols = position.width;
				auto w = formicine::perf.watch("Textbox::textAtRow");

				if (!allowWrap) {
					if (lines.size() <= size_t(row) + voffset)
						return pad_right? std::string(cols, ' ') + "\e[0m" : "\e[0m";
					LinePtr &line = lines.at(row + voffset);
					const std::string line_text = *line;
					const size_t line_length = ansi::length(line_text);
					if (cols < ssize_t(line_length))
						return ansi::substr(line_text, 0, cols) + "\e[0m";
					return line_text + (pad_right? std::string(cols - line_length, ' ') + "\e[0m": "\e[0m");
				}

				TextLine<C> *line;
				size_t offset;

				if (size_t(position.height) <= row || row < 0)
					return "\e[0m";

				if (lines.empty() || (row + voffset) >= totalRows()) {
					return pad_right? std::string(cols, ' ') + "\e[0m": "\e[0m";
				} else {
					std::tie(line, offset) = lineAtRow(row + voffset);
				}

				const std::string line_text = *line;
				const size_t continuation = line->getContinuation();

				if (offset == 0) {
					const size_t line_length = ansi::length(line_text);
					if (pad_right)
						return (ssize_t(line_length) <= cols? line_text + std::string(cols - line_length, ' ')
							: ansi::substr(line_text, 0, cols)) + "\e[0m";
					else
						return (ssize_t(line_length) <= cols? line_text : ansi::substr(line_text, 0, cols)) + "\e[0m";
				}

				// Number of chars visible per row on a continued line
				const size_t continuation_chars = cols - continuation;

				// Ignore the first line. 
				std::string str = ansi::substr(line_text, cols);

				// Erase the continued lines after the first line and before the continuation at the given row.
				ansi::erase(str, 0, (offset - 1) * continuation_chars);

				// Erase all characters after the continuation if any remain.
				if (continuation_chars < ansi::length(str))
					ansi::erase(str, continuation_chars, std::string::npos);

				const size_t str_length = ansi::length(str);
				// Return the continuation padding plus the segment of text visible on the row.
				if (pad_right && ssize_t(continuation + str_length) < cols)
					return std::string(continuation, ' ') + str + std::string(cols - continuation - str_length, ' ') +
						"\e[0m";
				return std::string(continuation, ' ') + str + "\e[0m";
			}

			/** Performs vertical scrolling for a given number of rows if autoscrolling is enabled and the right
			 *  conditions are met. This should be done after the line is added to the set of lines but before the line
			 *  is drawn. Returns true if this method caused any scrolling.*/
			bool doScroll(size_t rows) {
				if (autoscroll && position.height == ssize_t(totalRows()) - voffset) {
					vscroll(rows);
					return true;
				}

				return false;
			}

		public:
			/** The cached return value of total_rows(). */
			int totalRows_ = -1;

			/** The minimum number of lines that must be visible at the top. */
			unsigned int scrollBuffer = 0;

			/** Whether line wrapping is allowed. If false, overflowing text will be hidden, but performance will be
			 *  better. */
			bool allowWrap = true;

			/** Marks the cached return value of total_rows() as dirty. */
			void rowsDirty() {
				totalRows_ = -1;
			}

			/** Marks the num_rows_ values of the contained lines as dirty. */
			void linesDirty() {
				if (allowWrap)
					for (LinePtr &line: lines)
						line->markDirty();
			}

			/** Marks everything as dirty. */
			void markDirty() {
				rowsDirty();
				linesDirty();
			}

			Textbox(Container *parent_, const Position &pos_ = {}, bool allow_wrap = true,
				const C<std::string> &contents_ = {}):
				ColoredControl(parent_, pos_), allowWrap(allow_wrap) {
				if (parent_)
					parent_->addChild(this);
				setLines(contents_);
				// addChild could modify the position, so we set the position again at the end to overoverwrite it.
				position = pos_;
			}

			Textbox(Container *parent_, bool allow_wrap, const C<std::string> &contents_ = {}):
				ColoredControl(parent_), allowWrap(allow_wrap) {
				if (parent_)
					parent_->addChild(this);
				setLines(contents_);
			}

			/** Deletes all lines in the textbox. */
			void clearLines() {
				lines.clear();
				rowsDirty();
				if (0 < voffset)
					voffset = 0;
				draw();
			}

			C<LinePtr> & getLines() { return lines; }

			/** Scrolls the textbox down (positive argument) or up (negative argument). */
			void vscroll(ssize_t delta = 1) {
				auto w = formicine::perf.watch("Textbox::vscroll");

				const ssize_t total = totalRows();
				const ssize_t old_voffset = voffset;

				voffset = std::max(std::min(total - ssize_t(scrollBuffer), voffset + delta), 0l);

				// Don't let the voffset extend past the point where the (scrollBuffer + 1)th-last line of text is just
				// above the first row.
				if (position.height < total)
					voffset = std::min(voffset, total - static_cast<int>(scrollBuffer));

				if (!canDraw())
					return;

				auto lock = terminal->lockRender();
				const ssize_t diff = old_voffset - voffset;

				tryMargins([&, this]() {
					applyColors();
					terminal->vscroll(diff);

					// If new < old, we need to render newly exposed lines at the top. If old < new, we render at the
					// bottom.
					if (voffset < old_voffset) {
						terminal->jump(0, 0);
						for (ssize_t i = 0; i < diff; ++i) {
							*terminal << textAtRow(i);
							if (i < position.height - 1)
								*terminal << "\n";
						}
					} else if (old_voffset < voffset) {
						terminal->jump(0, position.height + diff);
						for (ssize_t i = position.height + std::max(diff, -position.height); i < position.height; ++i) {
							*terminal << textAtRow(i);
							if (i < position.height - 1)
								*terminal << "\n";
						}
					}

					uncolor();
				});

				terminal->jumpToFocused();
			}

			/** Returns the vertical offset. */
			int getVoffset() const {
				return voffset;
			}

			/** Sets the vertical offset. */
			void setVoffset(int voffset_) {
				const int old_voffset = voffset;
				if (voffset_ != old_voffset) {
					voffset = old_voffset; // TODO: what was I doing here?
					vscroll(voffset_ - old_voffset);
				}
			}

			bool getAutoscroll() const { return autoscroll; }

			void setAutoscroll(bool autoscroll_) {
				if (autoscroll != autoscroll_)
					autoscroll = autoscroll_;
			}

			/** Returns the number of rows on the terminal a line of text would occupy. */
			size_t lineRows(TextLine<C> &line) {
				// TODO: support doublewide characters.
				auto w = formicine::perf.watch("Textbox::lineRows");

				if (!allowWrap)
					return 1;

				auto lock = lockLines();
				return line.numRows(position.width);
			}

			/** Returns the total number of rows occupied by all the lines in the text box. */
			size_t totalRows() {
				auto w = formicine::perf.watch("Textbox::total_rows");
				auto lock = lockLines();

				if (!allowWrap)
					return lines.size();

				if (totalRows_ != -1)
					return totalRows_;

				totalRows_ = 0;
				for (const LinePtr &line: lines)
					totalRows_ += lineRows(*line);

				return totalRows_;
			}

			/** Draws the textbox on the terminal. */
			void draw() override {
				if (!canDraw())
					return;

				auto w = formicine::perf.watch("Textbox::draw");
				auto lock = terminal->lockRender();
				auto line_lock = lockLines();

				tryMargins([&, this]() {
					terminal->hide();
					clearRect();
					applyColors();

					if (0 <= voffset && ssize_t(totalRows()) <= voffset) {
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

			/** Resizes the textbox to fit a new position. */
			void resize(const Haunted::Position &new_pos) override {
				ColoredControl::resize(new_pos);
				markDirty();
			}

			/** Handles keyboard input. */
			bool onKey(const Key &key) override {
				return keyFunction? keyFunction(key) : defaultOnKey(key);
			}

			/** Calls the clicked textline's onMouse method. */
			bool onMouse(const MouseReport &report) override {
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
					TextLine<C> *line;
					std::tie(line, relative.y) = lineAtRow(relative.y + voffset);
					if (line) {
						line->onMouse(relative);
						return true;
					}
				} catch(const std::out_of_range &) {}

				return false;
			}

			/** Handles textbox-related keyboard input. */
			bool defaultOnKey(const Key &key) {
				if (key == KeyType::UpArrow) {
					vscroll(-1);
					draw();
				} else if (key == KeyType::DownArrow) {
					vscroll(1);
					draw();
				} else if (key == KeyType::LeftArrow) {
					setAutoscroll(true);
					draw();
				} else {
					return false;
				}

				return true;
			}

			bool canDraw() const override {
				return parent != nullptr && terminal != nullptr && !terminal->suppressOutput && !suppressDraw;
			}

			void focus() override {
				Control::focus();
				Colored::focus();
			}

			void redrawLine(TextLine<C> &to_redraw) {
				if (!canDraw())
					return;

				ssize_t rows = 0;
				auto lock = lockLines();
				for (LinePtr &line: lines) {
					if (line.get() == &to_redraw)
						break;
					rows += lineRows(*line);
				}

				ssize_t next = rows - voffset;
				if (voffset <= rows && next < position.height) {
					// The line is in view.
					to_redraw.markDirty();
					to_redraw.clean(position.width);
					const ssize_t new_lines = ssize_t(lineRows(to_redraw));
					tryMargins([this, &to_redraw, next, new_lines]() {
						applyColors();

						terminal->jump(0, next);
						for (ssize_t row = next, i = 0; row < position.height && i < new_lines; ++row, ++i) {
							if (0 < i)
								*terminal << "\n";
							*terminal << to_redraw.textAtRow(position.width, i, true);
						}

						uncolor();
					});
				}
			}

			/** Adds a string to the end of the textbox. */
			Textbox & operator+=(const std::string &text) {
				auto w = formicine::perf.watch("Textbox::operator+=");
				if (!text.empty() && text.back() == '\n')
					return *this += text.substr(0, text.size() - 1);

				std::shared_ptr<SimpleLine<C>> ptr = std::make_shared<SimpleLine<C>>(text, 0, &allowWrap);
				auto lock = lockLines();
				lines.push_back(std::move(ptr));
				rowsDirty();
				if (canDraw()) {
					if (autoscroll)
						doScroll(lines.back()->numRows(position.width));
					drawNewLine(*lines.back(), true);
				}
				return *this;
			}
			
			/** Adds a line to the end of the textbox. */
			template <EXTENDS(T, TextLine<C>)>
			Textbox & operator+=(T &line) {
				auto w = formicine::perf.watch("template Textbox::operator+=");
				std::unique_ptr<T> line_copy = std::make_unique<T>(line);
				line_copy->box = this;
				if (canDraw() && autoscroll)
					doScroll(line_copy->numRows(position.width));
				lines.push_back(std::move(line_copy));
				rowsDirty();
				if (canDraw())
					drawNewLine(*lines.back(), true);
				return *this;
			}

			/** Returns the textbox's contents. */
			operator std::string() {
				auto w = formicine::perf.watch("Textbox::operator std::string");
				auto lock = lockLines();
				std::string out = "";
				for (const LinePtr &line: lines) {
					if (!out.empty())
						out += "\n";
					out += std::string(*line);
				}

				return out;
			}

			virtual Terminal * getTerminal() override { return terminal; }
			virtual Container * getParent() const override { return parent; }

			size_t size() const { return lines.size(); }

			friend void swap(Textbox<C> &left, Textbox<C> &right) {
				swap(static_cast<Haunted::UI::Control &>(left), static_cast<Haunted::UI::Control &>(right));
				swap(static_cast<Haunted::UI::Colored &>(left), static_cast<Haunted::UI::Colored &>(right));
				std::swap(left.lines,        right.lines);
				std::swap(left.voffset,      right.voffset);
				std::swap(left.autoscroll,   right.autoscroll);
				std::swap(left.scrollBuffer, right.scrollBuffer);
				left.markDirty();
				right.markDirty();
			}
	};

	using DequeBox  = Textbox<std::deque>;
	using VectorBox = Textbox<std::vector>;
}
