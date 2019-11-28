#ifndef HAUNTED_UI_TEXTINPUT_H_
#define HAUNTED_UI_TEXTINPUT_H_

#include <cstddef>
#include <functional>
#include <ostream>
#include <string>
#include <unordered_set>

#include "haunted/core/defs.h"
#include "haunted/core/key.h"
#include "haunted/ui/colored.h"
#include "haunted/ui/container.h"
#include "haunted/ui/control.h"
#include "lib/ustring.h"

#include "lib/utf8.h"

namespace haunted::ui {
	/**
	 * Represents a control that accepts user input.
	 * This control should have a height of one row;
	 * any rows below the first will be unused.
	 */
	class textinput: public control, public colored {
		using update_fn = std::function<void(const ustring &, int)>;

		private:
			/** By default, all characters below 0x20 are ignored by insert(). However, if the character is contained in
			 *  this whitelist, it won't be ignored. */
			static std::unordered_set<unsigned char> whitelist;
			
			/** A string that's displayed at the left of the control before the contents of the buffer. */
			std::string prefix;

			/** The size in columns (not bytes) of the prefix. */
			size_t prefix_length = 0;

			/** The text that the user has entered so far. */
			ustring buffer;

			/** The offset within the text where new input will be inserted. */
			size_t cursor = 0;

			/** When the buffer contains more text than the control can display at once, some content will have to be
			 *  truncated. This field determines how many characters will be truncated on the left. */
			size_t scroll = 0;

			/** UTF-8 codepoints are received byte by byte. The first byte indicates how long the codepoint will be.
			 *  This field is set whenever we receive an indication of the codepoint length. */
			size_t bytes_expected = 0;

			/** A function to call whenever the buffer or cursor has changed. */
			update_fn on_update;

			/** A function to call whenever the buffer is submitted (e.g., the user presses return). */
			update_fn on_submit;

			/** Every time the textinput is redrawn, the screen position of the cursor is recorded. */
			point cursor_pos;

			/** Informs the update listener (if one has been added with listen()) that the buffer or cursor has changed.
			 */
			void update();

			/** Informs the submit listener (if one has been added with listen()) that the buffer has been submitted. */
			void submit();

			/** Partially re-renders the control onto the terminal in response to an cursor move. */
			void draw_cursor();

			/** Partially re-renders the control onto the terminal in response to an insertion. */
			void draw_insert();

			/** Partially re-renders the control onto the terminal in response to a deletion. */
			void draw_erase();

			/** Clears the space to the right of the cursor and redraws the text. */
			void draw_right(int offset = 0);

			/** Blanks out the spaces to the right of the buffer. */
			void clear_line();

			/** Blanks out the non-prefix portion of the textinput. */
			void clear_text();

			/** Blanks out the non-prefix portion of the textinput starting at a given offset past the left edge of the
			 *  non-prefix portion. */
			void clear_right(size_t);

			/** Returns a point representing the position of the textinput cursor on the screen. */
			point find_cursor() const;

			/** Adjusts the scroll if the input is too long and a character has been inserted, based on the cursor's
			 *  current position. Returns true if the scroll was changed. */
			bool check_scroll();

			/** Returns the character to the left of the cursor. */
			superchar prev_char();

			/** Returns the character to the right of the cursor. */
			superchar next_char();

			/** Returns the width of the buffer area (i.e., the width of the control minus the prefix length). */
			size_t text_width() const { return pos.width - prefix_length; }

			/** Returns true if the cursor is at the right edge of the textinput. */
			bool cursor_at_right() const { return cursor - scroll == text_width(); }

			/** Returns true if the cursor is at the left edge of the textinput. */
			bool cursor_at_left() const { return cursor == scroll; }

			/** Returns true if the cursor is at the end of the buffer. */
			bool at_end() const { return cursor == size(); }

		public:
			enum class event: int {update = 1, submit = 2};

			/** When a multibyte UTF-8 codepoint is being received, the individual bytes are stored in this buffer. */
			std::string unicode_buffer;

			/** Constructs a textinput with a parent and a position and an initial buffer and cursor. */
			textinput(container *parent, position pos, const ustring &buffer, size_t cursor);

			/** Constructs a textinput with a parent and a position and an initial buffer and a default cursor. */
			textinput(container *parent, position pos, const ustring &buffer):
				textinput(parent, pos, buffer, 0) {}

			/** Constructs a textinput with a parent and position and a default buffer and cursor. */
			textinput(container *parent, position pos):
				textinput(parent, pos, "") {}

			/** Constructs a textinput with a parent, a default position and an initial buffer and cursor. */
			textinput(container *parent, const ustring &buffer, size_t cursor);

			/** Constructs a textinput with a parent, a default position and an initial buffer and default cursor. */
			textinput(container *parent, const ustring &buffer):
				textinput(parent, buffer, 0) {}

			/** Constructs a textinput with a parent and a default position, buffer and cursor. */
			textinput(container *parent): textinput(parent, "") {}

			/** Constructs a textinput with no parent, no position and no initial contents. */
			textinput(): textinput(nullptr, "") {}

			/** Returns the contents of the buffer as a string. */
			std::string str() const { return buffer; }

			/** Converts the contents of the buffer to a string. */
			operator std::string() const { return buffer; }

			/** Sets a function to listen for updates to the buffer. */
			void listen(event, const update_fn &);

			/** Returns the cursor's offset. */
			size_t get_cursor() const { return cursor; }

			/** Moves the cursor to a given position. */
			void move_to(size_t);

			/** Inserts a string into the buffer at the cursor's position. */
			void insert(const std::string &);

			/** Inserts a single character into the buffer. */
			void insert(unsigned char);

			/** Erases the contents of the buffer and resets the cursor. */
			void clear();

			/** Erases the first word before the cursor (^w). */
			void erase_word();

			/** Erases the first character before the cursor. */
			void erase();

			/** Erases the first character after the cursor. */
			void erase_forward();

			/** Returns the contents of the buffer. */
			std::string get_text() const;

			/** Sets the contents of the buffer and moves the cursor to the end of the buffer. */
			void set_text(const std::string &);

			/** Sets the prefix portion of the textinput. */
			void set_prefix(const std::string &);

			/** Moves the cursor one character to the left unless it's already at the leftmost edge. */
			void left();

			/** Moves the cursor one character to the right unless it's already at the rightmost edge. */
			void right();

			/** Moves the cursor to the start of the buffer. */
			void start();

			/** Moves the cursor to the end of the buffer. */
			void end();

			/** Moves the cursor left by one word. Uses the same word-detecting logic as erase_word(). */
			void prev_word();

			/** Moves the cursor right by one word. Uses the same word-detecting logic as erase_word(). */
			void next_word();

			/** Swaps the character to the left of the cursor with the character to the right of the cursor. */
			void transpose();

			/** Returns the number of characters in the buffer. */
			size_t length() const;
			size_t size() const;

			/** Returns whether the buffer is empty. */
			bool empty() const;

			bool on_mouse(const mouse_report &) override;

			/** Handles key presses. */
			bool on_key(const key &) override;

			/** Renders the control onto the terminal. */
			virtual void draw() override;

			virtual bool can_draw() const override;

			/** Focuses the textinput and jumps to its cursor. */
			void focus() override;

			/** Moves the terminal cursor to the position of the textinput cursor. */
			void jump_cursor();

			/** Jumps to the cursor. */
			virtual void jump_focus() override;

			/** Moves the terminal cursor to the position of the textinput cursor if the textinput is focused. Returns
			 *  true if the textinput is focused and the cursor was moved. */
			bool try_jump();

			virtual terminal * get_terminal() override { return term; }
			virtual container * get_parent() const override { return parent; }

			/** Writes the contents of the buffer to an output stream. */
			friend std::ostream & operator<<(std::ostream &os, const textinput &input);
	};
}

#endif
