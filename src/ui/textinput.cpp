#include <iostream>
#include <stdexcept>

#include "formicine/ansi.h"
#include "utf8.h"

#include "core/terminal.h"
#include "ui/textinput.h"

namespace haunted::ui {
	std::unordered_set<unsigned char> textinput::whitelist = {9, 10, 11, 13};


// Private instance methods


	textinput::textinput(container *parent, position pos, const std::string &buffer, size_t cur):
	control(parent, pos), buffer(buffer), cursor(cur) {
		parent->add_child(this);
	}

	textinput::textinput(container *parent, const std::string &buffer, size_t cursor):
	control(parent), buffer(buffer), cursor(cursor) {
		parent->add_child(this);
	}

	void textinput::update() {
		if (on_update)
			on_update(buffer, cursor);
	}

	void textinput::draw_cursor() {
		if (term->has_focus(this))
			jump_cursor();
	}

	void textinput::draw_insert() {
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

		ansi::save();

		jump_cursor();
		ansi::left();
		point cpos = find_cursor();
		// Print only enough text to reach the right edge. Printing more would cause wrapping or text being printed out
		// of bounds.
		*term << buffer.substr(cur, pos.right() - cpos.x + 2);
		ansi::restore();
		if (has_focus())
			jump_cursor();
		flush();
	}

	void textinput::clear_line() {
		// If the buffer touches the right edge of the container, there's nothing to do.
		if (pos.width - prefix.length() > length() - scroll)
			clear_right(length() - scroll);
	}

	void textinput::clear_text() {
		clear_right(0);
	}

	void textinput::clear_right(size_t offset) {
		ansi::jump(pos.left + prefix.length() + offset, pos.top);
		if (at_right()) {
			// If we're bordering the right edge of the screen, we can clear everything to the right.
			ansi::clear_right();
		} else {
			*term << std::string(pos.width - (prefix.length() + offset), ' ');
			flush();
		}
	}

	point textinput::find_cursor() const {
		return {static_cast<int>(pos.left + prefix.length() + cursor - scroll), pos.top};
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

	utf8char textinput::prev_char() const {
		return cursor > 0? buffer[cursor - 1]  : utf8char();
	}

	utf8char textinput::next_char() const {
		return cursor < size()? buffer[cursor] : utf8char();
	}

	size_t textinput::text_width() const {
		return pos.width - prefix.length();
	}

	size_t textinput::get_cursor() const {
		return cursor;
	}

	bool textinput::cursor_at_right() const {
		return cursor - scroll == text_width();
	}

	bool textinput::cursor_at_left() const {
		return cursor == scroll;
	}

	bool textinput::at_end() const {
		return cursor == size();
	}


// Public instance methods


	void textinput::listen(const update_fn &fn) {
		on_update = fn;
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
		utf8str newstr = str;
		buffer.insert(cursor, newstr);
		cursor += newstr.size();
		update();
	}

	void textinput::insert(unsigned char ch) {
		if (ch < 0x20 && whitelist.find(ch) == whitelist.end())
			return;

		if (!unicode_buffer.empty()) {
			unicode_buffer.push_back(ch);
			if (unicode_buffer.size() == bytes_expected) {
				// The Unicode buffer now contains a complete and valid codepoint (the first byte is valid, at least).
				// Insert the buffer's contents into the primary buffer.
				buffer.insert(cursor++, unicode_buffer);
				unicode_buffer.clear();
				bytes_expected = 0;
				draw_insert();
				update();
			}
		} else {
			size_t width = utf8::width(ch);
			if (width < 2) {
				// It seems we've received a plain old ASCII character or an invalid UTF8 start byte.
				// Either way, append it to the buffer.
				buffer.insert(cursor++, ch);
				draw_insert();
				update();
			} else {
				// This byte is the first of a multi-byte codepoint. Set the expected width and initialize the Unicode
				// buffer with the byte.
				bytes_expected = width;
				unicode_buffer.push_back(ch);
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
		if (cursor == 0)
			return;
		size_t to_erase = 0;
		for (; prev_char() == ' '; --cursor)
			to_erase++;
		for (; prev_char() != '\0' && prev_char() != ' '; --cursor)
			to_erase++;
		buffer.erase(cursor, to_erase);
		check_scroll();
		draw_right();
		update();
	}

	void textinput::erase() {
		if (cursor > 0) {
			buffer.erase(--cursor, 1);
			if (cursor == length() && scroll < cursor && cursor - scroll < text_width()) {
				// If there's no text after the cursor and the cursor is in bounds, it should be sufficient to erase the
				// old character from the screen.
				ansi::save();
				jump_cursor();
				*term << ' ';
				ansi::restore();
			} else {
				// Otherwise, we need draw_erase() to handle things.
				draw_erase();
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

	void textinput::draw_right() {
		ansi::save();
		clear_line();
		jump_cursor();
		*term << buffer.substr(cursor, text_width() - cursor + scroll);
		ansi::restore();
	}

	void textinput::draw_erase() {
		if (text_width() <= cursor - scroll) {
			// If the cursor is at or beyond the right edge, do nothing.
			return;
		}

		ansi::save();

		if (cursor <= scroll) {
			// If the cursor is at or beyond the left edge, redraw the entire line.
			clear_text();
			ansi::jump(pos.left + prefix.length(), pos.top);
			*term << buffer.substr(scroll, text_width());
			ansi::restore();
			flush();
		} else {
			// If the cursor is somewhere between the two edges, clear part of the line and print part of the buffer.
			clear_right(cursor - scroll);
			jump_cursor();
			*term << buffer.substr(cursor, text_width() - cursor + scroll);
		}

		ansi::restore();
		flush();
	}

	std::string textinput::get_text() const {
		return buffer;
	}

	void textinput::set_text(const std::string &str) {
		buffer = icu::UnicodeString::fromUTF8(str);
		cursor = str.size();
		if (cursor > text_width())
			scroll = text_width() - cursor;
		draw();
		update();
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
		for (; prev_char() == ' '; --cursor);
		for (; prev_char() != '\0' && prev_char() != ' '; --cursor);
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
		for (; next_char() == ' '; ++cursor);
		for (; next_char() != '\0' && next_char() != ' '; ++cursor);
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

	size_t textinput::length() const {
		return buffer.length();
	}

	size_t textinput::size() const {
		return buffer.length();
	}

	bool textinput::on_key(key &k) {
		key_type type = k.type;
		key_modifier mod = k.mod;

		switch (mod) {
			case none:
				switch (type) {
					case right_arrow: right(); break;
					case left_arrow:   left(); break;
					case down_arrow:    end(); break;
					case up_arrow:    start(); break;
					case backspace:   erase(); break;
					case enter:       clear(); break;
					default:
						insert(char(k));
						if (check_scroll())
							draw();
						return true;
				}
				break;
			case ctrl:
				switch (type) {
					case 'a':      start(); break;
					case 'e':        end(); break;
					case 'u':      clear(); return true;
					case 'w': erase_word(); break;
					default: break;
				}
				break;
			case alt:
				switch (type) {
					case backspace: erase_word(); break;
					case 'b': prev_word();        break;
					case 'f': next_word();        break;
					case '~': erase_forward();    break;
					case 'H': start();            break;
					case 'F': end();              break;
					default: break;
				}
				break;
			default: break;
		}

		draw_cursor();
		flush();
		return true;
	}

	void textinput::draw() {
		size_t twidth = text_width();

		clear_line();
		jump();
		if (scroll < 0)
			throw std::domain_error("Scroll value cannot be negative.");

		*term << prefix << buffer.substr(scroll, twidth);
		jump_cursor();
	}

	void textinput::jump_cursor() {
		point cpos = find_cursor();
		ansi::jump(cpos.x, cpos.y);
	}

	bool textinput::try_jump() {
		if (!has_focus())
			return false;
		jump_cursor();
		return true;
	}

	textinput::operator std::string() const {
		return std::string(buffer);
	}

	std::ostream & operator<<(std::ostream &os, const textinput &input) {
		os << std::string(input);
		return os;
	}
}
