#include <iostream>
#include <stdexcept>

#include "haunted/core/Terminal.h"
#include "haunted/ui/TextInput.h"
#include "lib/UUtil.h"

namespace Haunted::UI {
	std::unordered_set<unsigned char> TextInput::whitelist = {9, 10, 11, 13};

	TextInput::TextInput(Container *parent_, const Position &pos_, const TextInput::String &buffer_, size_t cursor_):
	Control(parent_, pos_), buffer(buffer_), cursor(cursor_) {
		if (parent_)
			parent_->addChild(this);
	}

	TextInput::TextInput(Container *parent_, const TextInput::String &buffer_, size_t cursor_):
	Control(parent_), buffer(buffer_), cursor(cursor_) {
		if (parent_)
			parent_->addChild(this);
	}


// Private instance methods


	void TextInput::update() {
		if (onUpdate)
			onUpdate(buffer, cursor);
	}

	void TextInput::submit() {
		if (onSubmit)
			onSubmit(buffer, cursor);
	}

	void TextInput::drawCursor() {
		if (canDraw() && terminal->hasFocus(this))
			jumpCursor();
	}

	void TextInput::drawInsert(size_t count) {
		if (!canDraw())
			return;


		// It's assumed that the cursor has just been moved to the right from the insertion.
		// We need to account for that by using a decremented copy of the cursor.
		const size_t cur = cursor - count;

		if (textWidth() <= cur - scroll) {
			// If, for whatever reason, the cursor is to the right of the bounds of the TextInput, there's no visible
			// change to render because the change in text occurs entirely offscreen. We can just give up now if that's
			// the case.
			return;
		}

		if (cur < scroll) {
			// Things will turn out weird if we use this approach when the cursor is to the left of the bounds of the
			// TextInput, so when that's the case we just rerender the whole thing. TODO: improve this?
			draw();
			return;
		}

		auto lock = terminal->lockRender();
		tryColors();
		terminal->outStream.save();
		jumpCursor();
		terminal->outStream.left();
		// Point cpos = findCursor();
		// Print only enough text to reach the right edge. Printing more would cause wrapping or text being printed out
		// of bounds.
		// *terminal << buffer.substr(cur, position.right() - cpos.x + 2);
		// DBG("{{ \"" << buffer << "\"");
		// for (int i = 0; i < buffer.getData().length(); ++i) {
		// 	UChar ch = buffer.getData()[i];
		// 	DBG("  [" << std::hex << ch << std::dec << "]");
		// }
		// DBG("}}");

		printGraphemes(buffer.substr(cur));
		terminal->outStream.restore();
		terminal->colors.apply();
		if (hasFocus())
			jumpCursor();
		flush();
	}

	void TextInput::clearLine() {
		// If the buffer touches the right edge of the container, there's nothing to do.
		if (position.width - prefixLength > length() - scroll)
			clearRight(length() - scroll);
	}

	void TextInput::clearText() {
		clearRight(0);
	}

	void TextInput::clearRight(size_t offset) {
		if (!canDraw())
			return;

		terminal->jump(prefixLength + offset, position.top);
		applyColors();
		if (position.right() == terminal->getPosition().right()) {
			// If the TextInput stretches to the right edge of the terminal, we can use clearRight.
			terminal->outStream.clear_right();
		} else {
			// Horizontal margins don't work everywhere, and clearRight doesn't respect them anyway, so we have to use
			// this unsavory hack to clear just part of the screen.
			*terminal << std::string(position.width - (prefixLength + offset), ' ');
		}
	}

	Point TextInput::findCursor() const {
#ifndef ENABLE_ICU
		return {position.left + ssize_t(prefixLength) + ssize_t(cursor) - ssize_t(scroll), position.top};
#else
		return {position.left + ssize_t(prefixLength) + ssize_t(buffer.widthUntil(cursor, scroll)), position.top};
#endif
	}

	bool TextInput::checkScroll() {
		const size_t len = length();

		if (textWidth() < len && cursor == len) {
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
	char TextInput::prevChar() {
		return cursor > 0? buffer[cursor - 1]  : '\0';
	}

	char TextInput::nextChar() {
		return cursor < size()? buffer[cursor] : '\0';
	}
#else
	Superchar TextInput::prevChar() {
		return cursor > 0? buffer[cursor - 1]  : Superchar();
	}

	Superchar TextInput::nextChar() {
		return cursor < size()? buffer[cursor] : Superchar();
	}
#endif


// Public instance methods


	void TextInput::listen(Event event, const Update_f &fn) {
		if (event == Event::Update) {
			onUpdate = fn;
		} else if (event == Event::Submit) {
			onSubmit = fn;
		} else {
			throw std::invalid_argument("Invalid event type: " + std::to_string(int(event)));
		}
	}

	void TextInput::moveTo(size_t new_cursor) {
		if (new_cursor <= size() && new_cursor != cursor) {
			cursor = new_cursor;
			if (checkScroll()) {
				draw();
				tryJump();
			}
			update();
		}
	}

	void TextInput::insert(const std::string &str) {
		TextInput::String newstr = str;
		buffer.insert(cursor, newstr);
		cursor += ansi::length(newstr);
		update();
	}

	void TextInput::insert(unsigned char ch) {
		if (ch < 0x20 && whitelist.find(ch) == whitelist.end())
			return;

		if (!unicodeByteBuffer.empty()) {
			unicodeByteBuffer.push_back(ch);
			if (unicodeByteBuffer.size() == bytesExpected) {
				// The Unicode buffer now contains a complete and valid codepoint. (The first byte is valid, at least.)

				bool do_insert = true;

#ifdef ENABLE_ICU
				// Extract the full codepoint from the buffer.
				uint32_t uchar;
				UErrorCode code = U_ZERO_ERROR;
				icu::UnicodeString::fromUTF8(unicodeByteBuffer).toUTF32(reinterpret_cast<int *>(&uchar), 1, code);
				if (0 < code)
					throw std::runtime_error("TextInput::insert: toUTF32 returned " + std::to_string(code));


				// If this is half a flag...
				if (UUtil::isRegionalIndicator(uchar)) {
					if (unicodeCodepointBuffer.empty()) {
						// ...and it's the first half, don't insert it into the TextInput yet, but insert it into the
						// codepoint buffer.
						do_insert = false;
						unicodeCodepointBuffer.push_back(uchar);
					} else {
						// Otherwise, if it's the second half, insert both halves into the TextInput.
						// int uchars[] = {int(unicodeCodepointBuffer[0]), int(uchar)};
						int uchars[] = {int(unicodeCodepointBuffer[0]), int(uchar)};
						unicodeByteBuffer.clear();
						icu::UnicodeString::fromUTF32(uchars, 2).toUTF8String(unicodeByteBuffer);
						unicodeCodepointBuffer.clear();
					}
				}
#endif

				if (do_insert) { //-V547
					size_t old_length = buffer.length();
					DBG("Old length: " << old_length);
					buffer.insert(cursor, unicodeByteBuffer);
					DBG("New length: " << buffer.length());
					cursor += buffer.length() - old_length;
					DBG("Inserting character from Unicode buffer: \"" << unicodeByteBuffer << "\" (raw length: " <<
						unicodeByteBuffer.length() << ")");
					drawInsert();
					update();
				}

				unicodeByteBuffer.clear();
				bytesExpected = 0;
			}
		} else {
			const size_t width = UTF8::width(ch);
			if (width < 2) {
				// It seems we've received a plain old ASCII character or an invalid UTF8 start byte.
				// Either way, append it to the buffer.
#ifndef ENABLE_ICU
				buffer.insert(cursor++, 1, ch);
#else
				buffer.insert(cursor++, ch);
#endif
				drawInsert();
				update();
			} else {
				// This byte is the first of a multi-byte codepoint.
				// Set the expected width and initialize the Unicode buffer with the byte.
				bytesExpected = width;
				unicodeByteBuffer.push_back(ch);
			}
		}
	}

	void TextInput::clear() {
		buffer.clear();
		cursor = 0;
		clearText();
		jumpCursor();
		update();
	}

	void TextInput::eraseWord() {
		if (cursor == 0 || !canDraw())
			return;

		size_t to_erase = 0;
#ifndef ENABLE_ICU
		for (; prevChar() == ' '; --cursor)
			to_erase++;
		for (; prevChar() != '\0' && prevChar() != ' '; --cursor)
			to_erase++;
#else
		for (; prevChar() == " "; --cursor)
			to_erase++;
		for (; prevChar() != "" && prevChar() != " "; --cursor)
			to_erase++;
#endif
		buffer.erase(cursor, to_erase);
		checkScroll();
		drawRight();
		update();
	}

	void TextInput::erase() {
		if (cursor > 0) {
			auto lock = terminal->lockRender();
			buffer.erase(--cursor, 1);

			if (canDraw()) {
				if (cursor == length() && scroll < cursor && cursor - scroll < textWidth()) {
					// If there's no text after the cursor and the cursor is in bounds,
					// it should be sufficient to erase the old character from the screen.
					applyColors();
					terminal->outStream.save();
					jumpCursor();
					*terminal << ' ';
					terminal->outStream.restore();
				} else {
					// Otherwise, we need drawErase() to handle things.
					drawErase();
				}
			}

			update();
		}
	}

	void TextInput::eraseForward() {
		if (atEnd())
			return;

		buffer.erase(cursor, 1);
		drawErase();
		update();
	}

	std::string TextInput::getText() const {
		return buffer;
	}

	void TextInput::setText(const std::string &text) {
		buffer = text;
		cursor = text.size();
		if (cursor > textWidth())
			scroll = textWidth() - cursor;
		draw();
		update();
	}

	void TextInput::setPrefix(const std::string &prefix_) {
		if (prefix != prefix_) {
			prefix = prefix_;
			prefixLength = ansi::length(prefix);
			draw();
			update();
		}
	}

	void TextInput::left() {
		if (cursor == 0)
			return;

		if (cursor-- == scroll) {
			--scroll;
			draw();
		}
		
		update();
	}

	void TextInput::right() {
		// Don't do anything if the cursor is already at the end of the text.
		if (cursor == buffer.length())
			return;

		// Because there's text to the right, we increment the cursor.

		if (cursor++ - scroll == textWidth()) {
			// If we were at the right edge of the TextInput, we need to increment the scroll and redraw too.
			++scroll;
			draw();
		}

		update();
	}

	void TextInput::start() {
		if (cursor != 0) {
			cursor = 0;
			
			if (scroll != 0) {
				scroll = 0;
				draw();
			}

			update();
		}
	}

	void TextInput::end() {
		if (cursor != size()) {
			cursor = size();
			
			if (buffer.length() > textWidth()) {
				const size_t new_scroll = buffer.length() - textWidth();
				if (scroll != new_scroll) {
					scroll = new_scroll;
					draw();
				}
			}

			update();
		}
	}

	void TextInput::prevWord() {
		if (cursor == 0)
			return;

		const size_t old_cursor = cursor;

#ifndef ENABLE_ICU
		for (; prevChar() == ' '; --cursor);
		for (; prevChar() != '\0' && prevChar() != ' '; --cursor);
#else
		for (; prevChar() == " "; --cursor);
		for (; !prevChar().empty() && prevChar() != " "; --cursor);
#endif

		if (cursor != old_cursor) {
			if (cursor < scroll) {
				scroll = cursor;
				draw();
			}

			update();
		}
	}

	void TextInput::nextWord() {
		if (cursor == size())
			return;

		const size_t old_cursor = cursor;

#ifndef ENABLE_ICU
		for (; nextChar() == ' '; ++cursor);
		for (; nextChar() != '\0' && nextChar() != ' '; ++cursor);
#else
		for (; nextChar() == " "; ++cursor);
		for (; !nextChar().empty() && nextChar() != " "; ++cursor);
#endif

		if (cursor != old_cursor) {
			if (cursor - scroll > textWidth()) {
				// If we've moved beyond the right edge, we need to adjust the scroll to align the cursor with the right
				// edge.
				scroll = cursor - textWidth();
				draw();
			}

			update();
		}
	}

	void TextInput::transpose() {
		const size_t len = length();
		// If there aren't at least two characters, there's nothing to transpose.
		// Don't do anything if the cursor is at the beginning either.
		if (len < 2 || cursor == 0)
			return;

		if (cursor == len) {
			const size_t blen = buffer.length();
#ifndef ENABLE_ICU
			std::string penultimate = {1, buffer[blen - 2]}, ultimate = {1, buffer[blen - 1]};
#else
			std::string penultimate = buffer[blen - 2], ultimate = buffer[blen - 1];
#endif
			buffer.erase(blen - 2);
			buffer += ultimate + penultimate;
			drawRight(-2);
		} else {
#ifndef ENABLE_ICU
			std::string before_cursor = {1, buffer[cursor - 1]}, at_cursor = {1, buffer[cursor]};
#else
			std::string before_cursor = buffer[cursor - 1], at_cursor = buffer[cursor];
#endif
			buffer.erase(cursor - 1, 2);
			buffer.insert(cursor - 1, at_cursor + before_cursor);
			drawRight(-1);
			++cursor;
			jumpCursor();
		}
	}

	size_t TextInput::length() const {
		return buffer.length();
	}

	size_t TextInput::size() const {
		return buffer.length();
	}

	bool TextInput::empty() const {
		return buffer.empty();
	}

	bool TextInput::onMouse(const MouseReport &) {
		focus();
		return true;
	}

	bool TextInput::onKey(const Key &key) {
		const int type = int(key.type);
		ModSet mods = key.mods;

		switch (KeyMod(mods.to_ulong())) {
			case KeyMod::None:
				switch (type) {
					case int(KeyType::RightArrow): right(); break;
					case int(KeyType::LeftArrow):   left(); break;
					case int(KeyType::Backspace):   erase(); break;
					case int(KeyType::Enter):      submit(); break;
					case int(KeyType::Home):        start(); break;
					case int(KeyType::End):           end(); break;
					case int(KeyType::DownArrow):
					case int(KeyType::UpArrow):
					case int(KeyType::PageDown):
					case int(KeyType::PageUp): return false;
					case int(KeyType::Mouse):  return true;
					case int(KeyType::Tab):
						return false;
					default:
						insert(char(key));
						if (checkScroll())
							draw();
						return true;
				}
				break;
			case KeyMod::Ctrl:
				switch (type) {
					case 'a':     start(); break;
					case 'e':       end(); break;
					case 'h':     erase(); break;
					case 't': transpose(); break;
					case 'u':     clear(); break;
					case 'w': eraseWord(); break;
					case 'm': {
#ifndef ENABLE_ICU
						DBG("Length: " << buffer.length());
#else
						DBG("Length: " << buffer.length() << "; raw length: " << buffer.getData().length());
#endif
						for (size_t i = 0, len = buffer.length(); i < len; ++i) {
#ifndef ENABLE_ICU
							const std::string piece = {1, buffer[i]};
							DBG(i << ": " << "[" << piece << "] " << piece.length() << "l");
#else
							const std::string piece = buffer[i];
							DBG(i << ": " << "[" << piece << "] " << buffer.widthAt(i) << "w " << piece.length() << "l");
#endif
						}

#ifdef ENABLE_ICU
						buffer.scanLength();
						DBG("Scanned length. New length: " << buffer.length());
#endif
						break;
					}
					default: return false;
				}
				break;
			case KeyMod::Alt:
				switch (type) {
					case int(KeyType::Backspace): eraseWord(); break;
					case 'b': prevWord(); break;
					case 'f': nextWord(); break;
					case 'H': start();     break;
					case 'F': end();       break;
					default: return false;
				}
				break;
			default: return false;
		}

		drawCursor();
		flush();
		return true;
	}

	void TextInput::draw() {
		if (!canDraw())
			return;

		Colored::draw();

		auto lock = terminal->lockRender();
		size_t twidth = textWidth();

		clearLine();
		jump();

		try {
			// This is a kludge.
			ssize_t sscroll = scroll;
			if (sscroll < 0)
				sscroll = -sscroll;

			*terminal << prefix;
			printGraphemes(buffer.substr(sscroll));
		} catch (std::out_of_range &) {
			DBGT("std::out_of_range in TextInput::draw(): scroll[" << static_cast<ssize_t>(scroll) << "], twidth[" <<
				twidth << "], buffer[" << buffer.size() << "] = \"" << std::string(buffer) << "\"");
			*terminal << prefix;
		}

		terminal->resetColors();
		terminal->jumpToFocused();
	}

	void TextInput::drawRight(ssize_t offset) {
		if (!canDraw())
			return;

		auto lock = terminal->lockRender();
		applyColors();
		terminal->outStream.save();
		clearLine();
		const size_t old_cursor = cursor;
		cursor = offset < 0 && -offset > ssize_t(cursor)? 0 : cursor + offset;
		if (cursor > length())
			cursor = length();
		jumpCursor();
		// size_t twidth = textWidth();
		// *terminal << buffer.substr(cursor, twidth - cursor + scroll);
		printGraphemes(buffer.substr(cursor));
		cursor = old_cursor;
		terminal->outStream.restore();
	}

	void TextInput::drawErase() {
		if (!canDraw() || textWidth() <= cursor - scroll) {
			// If the cursor is at or beyond the right edge, do nothing.
			return;
		}

		auto lock = terminal->lockRender();
		applyColors();
		terminal->outStream.save();

		if (cursor <= scroll) {
			// If the cursor is at or beyond the left edge, redraw the entire line.
			clearText();
			terminal->jump(position.left + prefixLength, position.top);
			// *terminal << buffer.substr(scroll, textWidth());
			printGraphemes(buffer.substr(scroll));
			jumpCursor();
			flush();
		} else {
			// If the cursor is somewhere between the two edges, clear part of the line and print part of the buffer.
			clearRight(cursor - scroll);
			jumpCursor();
			// *terminal << buffer.substr(cursor, textWidth() - cursor + scroll);
			printGraphemes(buffer.substr(cursor));
		}

		terminal->outStream.restore();
		flush();
	}

	void TextInput::printGraphemes(TextInput::String to_print) {
		const size_t twidth = textWidth();

		String new_string;

		for (StringPiece piece: to_print) {
#ifdef ENABLE_ICU
			String promoted = String(piece);
#else
			String promoted = String(1, piece);
#endif
			for (auto &pair: characterRenderers)
				promoted = pair.second(promoted);
			new_string += promoted;
		}

#ifdef ENABLE_ICU
		while (twidth < new_string.width())
			new_string.pop_back();
#else
		while (twidth < new_string.size())
			new_string.pop_back();
#endif

#ifndef ENABLE_ICU
		for (const char grapheme: new_string) {
			*terminal << grapheme;
#else
		size_t i = 0;
		for (const std::string &grapheme: new_string) {
			const size_t width = new_string.widthAt(i++);
			if (width == 1) {
				*terminal << grapheme;
			} else {
				terminal->outStream.save();
				*terminal << grapheme;
				terminal->outStream.restore().right(width);
			}
#endif
		}
	}

	bool TextInput::canDraw() const {
		return Control::canDraw() && !terminal->suppressOutput;
	}

	void TextInput::focus() {
		Control::focus();
		Colored::focus();
		jumpCursor();
	}

	void TextInput::jumpCursor() {
		if (terminal) {
			auto lock = terminal->lockRender();
			Point cpos = findCursor();
			terminal->jump(cpos.x, cpos.y);
		}
	}

	void TextInput::jumpFocus() {
		jumpCursor();
	}

	bool TextInput::tryJump() {
		if (!terminal || !hasFocus())
			return false;
		jumpCursor();
		return true;
	}
}

std::ostream & operator<<(std::ostream &os, const Haunted::UI::TextInput &input) {
	return os << std::string(input);
}
