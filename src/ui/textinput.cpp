#include <iostream>

#include "lib/utf8.h"

#include "core/terminal.h"
#include "ui/textinput.h"

namespace haunted::ui {
	std::unordered_set<unsigned char> textinput::whitelist = {9, 10, 11, 13};

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

	void textinput::listen(const update_fn &fn) {
		on_update = fn;
	}

	void textinput::move_to(size_t new_cursor) {
		if (new_cursor <= size() && new_cursor != cursor) {
			cursor = new_cursor;
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
				// The Unicode buffer now contains a complete and valid codepoint
				// (the first byte is valid, at least).
				// Insert the buffer's contents into the primary buffer.
				buffer.insert(cursor++, unicode_buffer);
				unicode_buffer.clear();
				bytes_expected = 0;
				update();
			}
		} else {
			size_t width = utf8::width(ch);
			if (width < 2) {
				// It seems we've received a plain old ASCII
				// character or an invalid UTF8 start byte.
				// Either way, ppend it to the buffer.
				buffer.insert(cursor++, ch);
				update();
			} else {
				// This byte is the first of a multi-byte codepoint.
				// Set the expected width and initialize the Unicode buffer with the byte.
				bytes_expected = width;
				unicode_buffer.push_back(ch);
			}
		}
	}

	void textinput::clear() {
		buffer.clear();
		cursor = 0;
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
		update();
	}

	void textinput::erase() {
		if (cursor > 0) {
			buffer.erase(--cursor, 1);
			update();
		}
	}

	std::string textinput::get_text() const {
		return buffer;
	}

	void textinput::set_text(const std::string &str) {
		buffer = icu::UnicodeString::fromUTF8(str);
		cursor = str.size();
		update();
	}

	void textinput::left() {
		if (cursor > 0) {
			--cursor;
			update();
		}
	}

	void textinput::right() {
		if (cursor != buffer.length()) {
			++cursor;
			update();
		}
	}

	void textinput::start() {
		if (cursor != 0) {
			cursor = 0;
			update();
		}
	}

	void textinput::end() {
		if (cursor != size()) {
			cursor = size();
			update();
		}
	}

	void textinput::prev_word() {
		if (cursor == 0)
			return;
		size_t old_cursor = cursor;
		for (; prev_char() == ' '; --cursor);
		for (; prev_char() != '\0' && prev_char() != ' '; --cursor);
		if (cursor != old_cursor)
			update();
	}

	void textinput::next_word() {
		// if (cursor == size())
		// 	return;
		size_t old_cursor = cursor;
		for (; next_char() == ' '; ++cursor);
		for (; next_char() != '\0' && next_char() != ' '; ++cursor);
		if (cursor != old_cursor)
			update();
	}

	size_t textinput::length() const {
		return buffer.length();
	}

	size_t textinput::size() const {
		return buffer.length();
	}

	char textinput::prev_char() const {
		return cursor > 0? buffer[cursor - 1] : '\0';
	}

	char textinput::next_char() const {
		return cursor < size()? buffer[cursor] : '\0';
	}

	size_t textinput::get_cursor() const {
		return cursor;
	}

	bool textinput::on_key(key &k) {
		// std::cout << "type={" << k.type << "}, mod={" << k.mod << "}, k={" << k << "}\n";
		key_type type = k.type;
		switch (type) {
			case backspace: erase();   break;
			case left_arrow: left();   break;
			case right_arrow: right(); break;
			default:
				if (k.mod == none)
					insert(char(k));
		}

		term->redraw();
		std::cout.flush();
		return true;
	}

	void textinput::draw() {
		size_t remaining = buffer.length() - prefix.length();
		size_t width = pos.width;

		if (size() <= width - prefix.length()) {
			jump();
			std::cout << prefix << buffer;
			return;
		}
	}

	textinput::operator std::string() const {
		return std::string(buffer);
	}

	std::ostream & operator<<(std::ostream &os, const textinput &input) {
		os << std::string(input);
		return os;
	}
}
