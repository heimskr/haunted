#include <iostream>
#include <stdexcept>

#include "haunted/core/terminal.h"
#include "haunted/ui/textinput.h"
#include "lib/uutil.h"

namespace haunted::ui {
	std::unordered_set<unsigned char> textinput::whitelist = {9, 10, 11, 13};

	textinput::textinput(container *parent_, position pos_, const textinput::string &buffer_, size_t cursor_):
	control(parent_, pos_), buffer(buffer_), cursor(cursor_) {
		if (parent_ != nullptr)
			parent_->add_child(this);
	}

	textinput::textinput(container *parent_, const textinput::string &buffer_, size_t cursor_):
	control(parent_), buffer(buffer_), cursor(cursor_) {
		if (parent_ != nullptr)
			parent_->add_child(this);
	}


// Private instance methods


	void textinput::update() {
		if (on_update)
			on_update(buffer, cursor);
	}

	void textinput::submit() {
		if (on_submit)
			on_submit(buffer, cursor);
	}

	void textinput::draw_cursor() {
		if (can_draw() && term->has_focus(this))
			jump_cursor();
	}

	void textinput::draw_insert() {
		if (!can_draw())
			return;

		// It's assumed that the cursor has just been moved to the right from the insertion.
		// We need to account for that by using a decremented copy of the cursor.
		size_t cur = cursor - 1;

		if (text_width() <= cur - scroll) {
			// If, for whatever reason, the cursor is to the right of the bounds of the textinput, there's no visible
			// change to render because the change in text occurs entirely offscreen. We can just give up now if that's
			// the case.
			return;
		}

		if (cur < scroll) {
			// Things will turn out weird if we use this approach when the cursor is to the left of the bounds of the
			// textinput, so when that's the case we just rerender the whole thing. TODO: improve this?
			draw();
			return;
		}

		auto lock = term->lock_render();
		try_colors();
		term->out_stream.save();
		jump_cursor();
		term->out_stream.left();
		point cpos = find_cursor();
		// Print only enough text to reach the right edge. Printing more would cause wrapping or text being printed out
		// of bounds.
		*term << buffer.substr(cur, pos.right() - cpos.x + 2);
		term->out_stream.restore();
		term->colors.apply();
		if (has_focus())
			jump_cursor();
		flush();
	}

	void textinput::clear_line() {
		// If the buffer touches the right edge of the container, there's nothing to do.
		if (pos.width - prefix_length > length() - scroll)
			clear_right(length() - scroll);
	}

	void textinput::clear_text() {
		clear_right(0);
	}

	void textinput::clear_right(size_t offset) {
		if (!can_draw())
			return;

		term->jump(prefix_length + offset, pos.top);
		apply_colors();
		if (pos.right() == term->get_position().right()) {
			// If the textinput stretches to the right edge of the terminal, we can use clear_right.
			term->out_stream.clear_right();
		} else {
			// Horizontal margins don't work everywhere, and clear_right doesn't respect them anyway, so we have to use
			// this unsavory hack to clear just part of the screen.
			*term << std::string(pos.width - (prefix_length + offset), ' ');
		}
	}

	point textinput::find_cursor() const {
#ifndef ENABLE_ICU
		return {static_cast<int>(pos.left + prefix_length + cursor - scroll), pos.top};
#else
		return {static_cast<int>(pos.left + prefix_length + buffer.width_until(cursor, scroll)), pos.top};
#endif
	}

	bool textinput::check_scroll() {
		const size_t len = length();

		if (text_width() < len && cursor == len) {
			// If the cursor is at the end of the input, increment the scroll. This keeps the cursor at the right edge
			// of the control and pushes the other text to the left, truncating the first character.
			++scroll;
			return true;
		} else if (cursor < scroll) {
			// If the cursor is to the left of the screen, decrement the scroll to align the cursor with the start of
			// the non-prefix portion.
			scroll = cursor;
			return true;
		}

		return false;
	}

#ifndef ENABLE_ICU
	char textinput::prev_char() {
		return cursor > 0? buffer[cursor - 1]  : '\0';
	}

	char textinput::next_char() {
		return cursor < size()? buffer[cursor] : '\0';
	}
#else
	superchar textinput::prev_char() {
		return cursor > 0? buffer[cursor - 1]  : superchar();
	}

	superchar textinput::next_char() {
		return cursor < size()? buffer[cursor] : superchar();
	}
#endif


// Public instance methods


	void textinput::listen(event evt, const update_fn &fn) {
		if (evt == event::update) {
			on_update = fn;
		} else if (evt == event::submit) {
			on_submit = fn;
		} else {
			throw std::invalid_argument("Invalid event type: " + std::to_string(static_cast<int>(evt)));
		}
	}

	void textinput::move_to(size_t new_cursor) {
		if (new_cursor <= size() && new_cursor != cursor) {
			cursor = new_cursor;
			if (check_scroll()) {
				draw();
				try_jump();
			}
			update();
		}
	}

	void textinput::insert(const std::string &str) {
		textinput::string newstr = str;
		buffer.insert(cursor, newstr);
		cursor += ansi::length(newstr);
		update();
	}

	void textinput::insert(unsigned char ch) {
		if (ch < 0x20 && whitelist.find(ch) == whitelist.end())
			return;

		if (!unicode_byte_buffer.empty()) {
			unicode_byte_buffer.push_back(ch);
			if (unicode_byte_buffer.size() == bytes_expected) {
				// The Unicode buffer now contains a complete and valid codepoint. (The first byte is valid, at least.)

				bool do_insert = true;

#ifdef ENABLE_ICU
				// Extract the full codepoint from the buffer.
				uint32_t uchar;
				UErrorCode code = U_ZERO_ERROR;
				icu::UnicodeString::fromUTF8(unicode_byte_buffer).toUTF32(reinterpret_cast<int *>(&uchar), 1, code);
				if (0 < code)
					throw std::runtime_error("textinput::insert: toUTF32 returned " + std::to_string(code));


				// If this is half a flag...
				if (uutil::is_regional_indicator(uchar)) {
					if (unicode_codepoint_buffer.empty()) {
						// ...and it's the first half, don't insert it into the textinput yet, but insert it into the
						// codepoint buffer.
						do_insert = false;
						unicode_codepoint_buffer.push_back(uchar);
					} else {
						// Otherwise, if it's the second half, insert both halves into the textinput.
						// int uchars[] = {static_cast<int>(unicode_codepoint_buffer[0]), static_cast<int>(uchar)};
						int uchars[] = {static_cast<int>(unicode_codepoint_buffer[0]), static_cast<int>(uchar)};
						unicode_byte_buffer.clear();
						icu::UnicodeString::fromUTF32(uchars, 2).toUTF8String(unicode_byte_buffer);
						unicode_codepoint_buffer.clear();
					}
				}
#endif

				if (do_insert) {
					size_t old_length = buffer.length();
					DBG("Old length: " << old_length);
					buffer.insert(cursor, unicode_byte_buffer);
					DBG("New length: " << buffer.length());
					cursor += buffer.length() - old_length;
					DBG("Inserting character from Unicode buffer: \"" << unicode_byte_buffer << "\" (raw length: " <<
						unicode_byte_buffer.length() << ")");
					draw_insert();
					update();
				}

				unicode_byte_buffer.clear();
				bytes_expected = 0;
			}
		} else {
			size_t width = utf8::width(ch);
			if (width < 2) {
				// It seems we've received a plain old ASCII character or an invalid UTF8 start byte.
				// Either way, append it to the buffer.
#ifndef ENABLE_ICU
				buffer.insert(cursor++, 1, ch);
#else
				buffer.insert(cursor++, ch);
#endif
				draw_insert();
				update();
			} else {
				// This byte is the first of a multi-byte codepoint.
				// Set the expected width and initialize the Unicode buffer with the byte.
				bytes_expected = width;
				unicode_byte_buffer.push_back(ch);
			}
		}
	}

	void textinput::clear() {
		buffer.clear();
		cursor = 0;
		clear_text();
		jump_cursor();
		update();
	}

	void textinput::erase_word() {
		if (cursor == 0 || !can_draw())
			return;

		size_t to_erase = 0;
#ifndef ENABLE_ICU
		for (; prev_char() == ' '; --cursor)
			to_erase++;
		for (; prev_char() != '\0' && prev_char() != ' '; --cursor)
			to_erase++;
#else
		for (; prev_char() == " "; --cursor)
			to_erase++;
		for (; prev_char() != "" && prev_char() != " "; --cursor)
			to_erase++;
#endif
		buffer.erase(cursor, to_erase);
		check_scroll();
		draw_right();
		update();
	}

	void textinput::erase() {
		if (cursor > 0) {
			auto lock = term->lock_render();
			buffer.erase(--cursor, 1);

			if (can_draw()) {
				if (cursor == length() && scroll < cursor && cursor - scroll < text_width()) {
					// If there's no text after the cursor and the cursor is in bounds,
					// it should be sufficient to erase the old character from the screen.
					apply_colors();
					term->out_stream.save();
					jump_cursor();
					*term << ' ';
					term->out_stream.restore();
				} else {
					// Otherwise, we need draw_erase() to handle things.
					draw_erase();
				}
			}

			update();
		}
	}

	void textinput::erase_forward() {
		if (at_end())
			return;

		buffer.erase(cursor, 1);
		draw_erase();
		update();
	}

	std::string textinput::get_text() const {
		return buffer;
	}

	void textinput::set_text(const std::string &text) {
		buffer = text;
		cursor = text.size();
		if (cursor > text_width())
			scroll = text_width() - cursor;
		draw();
		update();
	}

	void textinput::set_prefix(const std::string &prefix_) {
		if (prefix != prefix_) {
			prefix = prefix_;
			prefix_length = ansi::length(prefix);
			draw();
			update();
		}
	}

	void textinput::left() {
		if (cursor == 0)
			return;

		if (cursor-- == scroll) {
			--scroll;
			draw();
		}
		
		update();
	}

	void textinput::right() {
		// Don't do anything if the cursor is already at the end of the text.
		if (cursor == buffer.length())
			return;

		// Because there's text to the right, we increment the cursor.

		if (cursor++ - scroll == text_width()) {
			// If we were at the right edge of the textinput, we need to increment the scroll and redraw too.
			++scroll;
			draw();
		}

		update();
	}

	void textinput::start() {
		if (cursor != 0) {
			cursor = 0;
			
			if (scroll != 0) {
				scroll = 0;
				draw();
			}

			update();
		}
	}

	void textinput::end() {
		if (cursor != size()) {
			cursor = size();
			
			if (buffer.length() > text_width()) {
				size_t new_scroll = buffer.length() - text_width();
				if (scroll != new_scroll) {
					scroll = new_scroll;
					draw();
				}
			}

			update();
		}
	}

	void textinput::prev_word() {
		if (cursor == 0)
			return;

		size_t old_cursor = cursor;

#ifndef ENABLE_ICU
		for (; prev_char() == ' '; --cursor);
		for (; prev_char() != '\0' && prev_char() != ' '; --cursor);
#else
		for (; prev_char() == " "; --cursor);
		for (; !prev_char().empty() && prev_char() != " "; --cursor);
#endif

		if (cursor != old_cursor) {
			if (cursor < scroll) {
				scroll = cursor;
				draw();
			}

			update();
		}
	}

	void textinput::next_word() {
		if (cursor == size())
			return;

		size_t old_cursor = cursor;

#ifndef ENABLE_ICU
		for (; next_char() == ' '; ++cursor);
		for (; next_char() != '\0' && next_char() != ' '; ++cursor);
#else
		for (; next_char() == " "; ++cursor);
		for (; !next_char().empty() && next_char() != " "; ++cursor);
#endif

		if (cursor != old_cursor) {
			if (cursor - scroll > text_width()) {
				// If we've moved beyond the right edge, we need to adjust the scroll to align the cursor with the right
				// edge.
				scroll = cursor - text_width();
				draw();
			}

			update();
		}
	}

	void textinput::transpose() {
		const size_t len = length();
		// If there aren't at least two characters, there's nothing to transpose.
		// Don't do anything if the cursor is at the beginning either.
		if (len < 2 || cursor == 0)
			return;

		if (cursor == len) {
			size_t blen = buffer.length();
#ifndef ENABLE_ICU
			std::string penultimate = {1, buffer[blen - 2]}, ultimate = {1, buffer[blen - 1]};
#else
			std::string penultimate = buffer[blen - 2], ultimate = buffer[blen - 1];
#endif
			buffer.erase(blen - 2);
			buffer += ultimate + penultimate;
			draw_right(-2);
		} else {
#ifndef ENABLE_ICU
			std::string before_cursor = {1, buffer[cursor - 1]}, at_cursor = {1, buffer[cursor]};
#else
			std::string before_cursor = buffer[cursor - 1], at_cursor = buffer[cursor];
#endif
			buffer.erase(cursor - 1, 2);
			buffer.insert(cursor - 1, at_cursor + before_cursor);
			draw_right(-1);
			++cursor;
			jump_cursor();
		}
	}

	size_t textinput::length() const {
		return buffer.length();
	}

	size_t textinput::size() const {
		return buffer.length();
	}

	bool textinput::empty() const {
		return buffer.empty();
	}

	bool textinput::on_mouse(const mouse_report &) {
		focus();
		return true;
	}

	bool textinput::on_key(const key &k) {
		int type = int(k.type);
		modset mods = k.mods;

		switch (kmod(mods.to_ulong())) {
			case kmod::none:
				switch (type) {
					case int(ktype::right_arrow): right(); break;
					case int(ktype::left_arrow):   left(); break;
					case int(ktype::backspace):   erase(); break;
					case int(ktype::enter):      submit(); break;
					case int(ktype::home):        start(); break;
					case int(ktype::end):           end(); break;
					case int(ktype::down_arrow):
					case int(ktype::up_arrow):
					case int(ktype::page_down):
					case int(ktype::page_up): return false;
					case int(ktype::mouse):   return true;
					case int(ktype::tab):
						return false;
					default:
						insert(char(k));
						if (check_scroll())
							draw();
						return true;
				}
				break;
			case kmod::ctrl:
				switch (type) {
					case 'a':      start(); break;
					case 'e':        end(); break;
					case 'h':      erase(); break;
					case 't':  transpose(); break;
					case 'u':      clear(); break;
					case 'w': erase_word(); break;
					case 'm': {
#ifndef ENABLE_ICU
						DBG("Length: " << buffer.length());
#else
						DBG("Length: " << buffer.length() << "; raw length: " << buffer.get_data().length());
#endif
						for (size_t i = 0, len = buffer.length(); i < len; ++i) {
#ifndef ENABLE_ICU
							std::string piece = {1, buffer[i]};
							DBG(i << ": " << "[" << piece << "] " << piece.length() << "l");
#else
							std::string piece = buffer[i];
							DBG(i << ": " << "[" << piece << "] " << buffer.width_at(i) << "w " << piece.length() << "l");
#endif
						}

#ifdef ENABLE_ICU
						buffer.scan_length();
						DBG("Scanned length. New length: " << buffer.length());
#endif
						break;
					}
					default: return false;
				}
				break;
			case kmod::alt:
				switch (type) {
					case int(ktype::backspace): erase_word(); break;
					case 'b': prev_word(); break;
					case 'f': next_word(); break;
					case 'H': start();     break;
					case 'F': end();       break;
					default: return false;
				}
				break;
			default: return false;
		}

		draw_cursor();
		flush();
		return true;
	}

	void textinput::draw() {
		if (!can_draw())
			return;

		colored::draw();

		auto lock = term->lock_render();
		size_t twidth = text_width();

		clear_line();
		jump();

		try {
			// This is a kludge.
			ssize_t sscroll = scroll;
			if (sscroll < 0)
				sscroll = -sscroll;

			*term << prefix;
			print_graphemes(buffer.substr(sscroll));
		} catch (std::out_of_range &) {
			DBGT("std::out_of_range in textinput::draw(): scroll[" << static_cast<ssize_t>(scroll) << "], twidth[" <<
				twidth << "], buffer[" << buffer.size() << "] = \"" << std::string(buffer) << "\"");
			*term << prefix;
		}

		term->reset_colors();
		term->jump_to_focused();
	}

	void textinput::draw_right(int offset) {
		if (!can_draw())
			return;

		auto lock = term->lock_render();
		DBG("draw_right(" << offset << ")");
		apply_colors();
		term->out_stream.save();
		clear_line();
		size_t old_cursor = cursor;
		cursor = offset < 0 && -offset > static_cast<int>(cursor)? 0 : cursor + offset;
		if (cursor > length())
			cursor = length();
		jump_cursor();
		size_t twidth = text_width();
		*term << buffer.substr(cursor, twidth - cursor + scroll);
		cursor = old_cursor;
		term->out_stream.restore();
	}

	void textinput::draw_erase() {
		DBG("draw_erase()");
		if (!can_draw() || text_width() <= cursor - scroll) {
			// If the cursor is at or beyond the right edge, do nothing.
			return;
		}

		auto lock = term->lock_render();
		apply_colors();
		term->out_stream.save();

		if (cursor <= scroll) {
			// If the cursor is at or beyond the left edge, redraw the entire line.
			clear_text();
			term->jump(pos.left + prefix_length, pos.top);
			// *term << buffer.substr(scroll, text_width());
			print_graphemes(buffer.substr(scroll));
			jump_cursor();
			flush();
		} else {
			// If the cursor is somewhere between the two edges, clear part of the line and print part of the buffer.
			clear_right(cursor - scroll);
			jump_cursor();
			// *term << buffer.substr(cursor, text_width() - cursor + scroll);
			print_graphemes(buffer.substr(cursor));
		}

		term->out_stream.restore();
		flush();
	}

	void textinput::print_graphemes(textinput::string to_print) {
		size_t twidth = text_width();
#ifndef ENABLE_ICU
		size_t width = to_print.size();
#else
		size_t width = to_print.width();
#endif
		while (twidth < width)
			to_print.pop_back();

#ifndef ENABLE_ICU
		for (const char grapheme: to_print) {
			*term << grapheme;
#else
		size_t i = 0;
		for (const std::string &grapheme: to_print) {
			width = to_print.width_at(i++);
			if (width == 1) {
				*term << grapheme;
			} else {
				term->out_stream.save();
				*term << grapheme;
				term->out_stream.restore().right(width);
			}
#endif
		}
	}

	bool textinput::can_draw() const {
		return control::can_draw() && !term->suppress_output;
	}

	void textinput::focus() {
		control::focus();
		colored::focus();
		jump_cursor();
	}

	void textinput::jump_cursor() {
		if (term != nullptr) {
			auto lock = term->lock_render();
			point cpos = find_cursor();
			term->jump(cpos.x, cpos.y);
		}
	}

	void textinput::jump_focus() {
		jump_cursor();
	}

	bool textinput::try_jump() {
		if (term == nullptr || !has_focus())
			return false;
		jump_cursor();
		return true;
	}

	std::ostream & operator<<(std::ostream &os, const textinput &input) {
		os << std::string(input);
		return os;
	}
}
