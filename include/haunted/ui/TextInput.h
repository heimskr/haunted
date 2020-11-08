#ifndef HAUNTED_UI_TEXTINPUT_H_
#define HAUNTED_UI_TEXTINPUT_H_

#include <cstddef>
#include <functional>
#include <list>
#include <ostream>
#include <string>
#include <unordered_set>

#include "haunted/core/Defs.h"
#include "haunted/core/Key.h"
#include "haunted/ui/Colored.h"
#include "haunted/ui/Container.h"
#include "haunted/ui/Control.h"
#include "lib/ustring.h"

#include "lib/UTF8.h"

namespace Haunted::UI {
	/**
	 * Represents a control that accepts user input.
	 * This control should have a height of one row;
	 * any rows below the first will be unused.
	 */
	class TextInput: public Control, public Colored {
		public:
#ifndef ENABLE_ICU
			using String = std::string;
			using StringPiece = char;
#else
			using String = ustring;
			using StringPiece = std::string;
#endif
			using Update_f = std::function<void(const String &, int)>;
			using RenderChar_f = std::function<String(const String &)>;

		private:
			/** By default, all characters below 0x20 are ignored by insert(). However, if the character is contained in
			 *  this whitelist, it won't be ignored. */
			static std::unordered_set<unsigned char> whitelist;
			
			/** A string that's displayed at the left of the control before the contents of the buffer. */
			std::string prefix;

			/** The size in columns (not bytes) of the prefix. */
			size_t prefixLength = 0;

			/** The text that the user has entered so far. */
			String buffer;

			/** The offset within the text where new input will be inserted. */
			size_t cursor = 0;

			/** When the buffer contains more text than the control can display at once, some content will have to be
			 *  truncated. This field determines how many characters will be truncated on the left. */
			size_t scroll = 0;

			/** UTF-8 codepoints are received byte by byte. The first byte indicates how long the codepoint will be.
			 *  This field is set whenever we receive an indication of the codepoint length. */
			size_t bytesExpected = 0;

			/** A function to call whenever the buffer or cursor has changed. */
			Update_f onUpdate;

			/** A function to call whenever the buffer is submitted (e.g., the user presses return). */
			Update_f onSubmit;

			/** Every time the TextInput is redrawn, the screen position of the cursor is recorded. */
			Point cursorPosition;

			/** Informs the update listener (if one has been added with listen()) that the buffer or cursor has changed.
			 */
			void update();

			/** Informs the submit listener (if one has been added with listen()) that the buffer has been submitted. */
			void submit();

			/** Blanks out the spaces to the right of the buffer. */
			void clearLine();

			/** Blanks out the non-prefix portion of the TextInput. */
			void clearText();

			/** Blanks out the non-prefix portion of the TextInput starting at a given offset past the left edge of the
			 *  non-prefix portion. */
			void clearRight(size_t);

			/** Returns a point representing the position of the TextInput cursor on the screen. */
			Point findCursor() const;

			/** Adjusts the scroll if the input is too long and a character has been inserted, based on the cursor's
			 *  current position. Returns true if the scroll was changed. */
			bool checkScroll();

#ifndef ENABLE_ICU
			/** Returns the character to the left of the cursor. */
			char prevChar();

			/** Returns the character to the right of the cursor. */
			char nextChar();
#else
			/** Returns the character to the left of the cursor. */
			superchar prevChar();

			/** Returns the character to the right of the cursor. */
			superchar nextChar();
#endif

			/** Returns the width of the buffer area (i.e., the width of the control minus the prefix length). */
			size_t textWidth() const { return position.width - prefixLength; }

			/** Returns true if the cursor is at the right edge of the TextInput. */
			bool cursorAtRight() const { return cursor - scroll == textWidth(); }

			/** Returns true if the cursor is at the left edge of the TextInput. */
			bool cursorAtLeft() const { return cursor == scroll; }

			/** Returns true if the cursor is at the end of the buffer. */
			bool atEnd() const { return cursor == size(); }

		public:
			enum class Event: int {Update = 1, Submit = 2};

			/** When a multibyte UTF-8 codepoint is being received, the individual bytes are stored in this buffer. */
			std::string unicodeByteBuffer;

			/** When a multi-codepoint UTF-8 grapheme is being received, the individual codepoints are stored in this
			 *  buffer. */
			std::vector<uint32_t> unicodeCodepointBuffer;

			/** Contains functions that change how individual characters should be rendered. */
			std::map<std::string, RenderChar_f> characterRenderers;

			/** Constructs a TextInput with a parent and a position and an initial buffer and cursor. */
			TextInput(Container *parent, const Position &pos, const String &buffer, size_t cursor);

			/** Constructs a TextInput with a parent and a position and an initial buffer and a default cursor. */
			TextInput(Container *parent, const Position &pos, const String &buffer):
				TextInput(parent, pos, buffer, 0) {}

			/** Constructs a TextInput with a parent and position and a default buffer and cursor. */
			TextInput(Container *parent, const Position &pos):
				TextInput(parent, pos, "") {}

			/** Constructs a TextInput with a parent, a default position and an initial buffer and cursor. */
			TextInput(Container *parent, const String &buffer, size_t cursor);

			/** Constructs a TextInput with a parent, a default position and an initial buffer and default cursor. */
			TextInput(Container *parent, const String &buffer):
				TextInput(parent, buffer, 0) {}

			/** Constructs a TextInput with a parent and a default position, buffer and cursor. */
			TextInput(Container *parent): TextInput(parent, "") {}

			/** Constructs a TextInput with no parent, no position and no initial contents. */
			TextInput(): TextInput(nullptr, "") {}

			/** Returns the contents of the buffer as a string. */
			std::string str() const { return buffer; }

			/** Converts the contents of the buffer to a string. */
			operator std::string() const { return buffer; }

			/** Sets a function to listen for updates to the buffer. */
			void listen(Event, const Update_f &);

			/** Returns the cursor's offset. */
			size_t getCursor() const { return cursor; }

			/** Moves the cursor to a given position. */
			void moveTo(size_t);

			/** Inserts a string into the buffer at the cursor's position. */
			void insert(const std::string &);

			/** Inserts a single character into the buffer. */
			void insert(unsigned char);

			/** Erases the contents of the buffer and resets the cursor. */
			void clear();

			/** Erases the first word before the cursor (^w). */
			void eraseWord();

			/** Erases the first character before the cursor. */
			void erase();

			/** Erases the first character after the cursor. */
			void eraseForward();

			/** Returns the contents of the buffer. */
			std::string getText() const;

			/** Sets the contents of the buffer and moves the cursor to the end of the buffer. */
			void setText(const std::string &);

			/** Sets the prefix portion of the TextInput. */
			void setPrefix(const std::string &);

			/** Moves the cursor one character to the left unless it's already at the leftmost edge. */
			void left();

			/** Moves the cursor one character to the right unless it's already at the rightmost edge. */
			void right();

			/** Moves the cursor to the start of the buffer. */
			void start();

			/** Moves the cursor to the end of the buffer. */
			void end();

			/** Moves the cursor left by one word. Uses the same word-detecting logic as eraseWord(). */
			void prevWord();

			/** Moves the cursor right by one word. Uses the same word-detecting logic as eraseWord(). */
			void nextWord();

			/** Swaps the character to the left of the cursor with the character to the right of the cursor. */
			void transpose();

			/** Returns the number of characters in the buffer. */
			size_t length() const;
			size_t size() const;

			/** Returns whether the buffer is empty. */
			bool empty() const;

			bool onMouse(const MouseReport &) override;

			/** Handles key presses. */
			bool onKey(const Key &) override;

			/** Renders the control onto the terminal. */
			virtual void draw() override;

			/** Clears the space to the right of the cursor and redraws the text. */
			void drawRight(int offset = 0);

			/** Partially re-renders the control onto the terminal in response to a deletion. */
			void drawErase();

			/** Partially re-renders the control onto the terminal in response to an cursor move. */
			void drawCursor();

			/** Partially re-renders the control onto the terminal in response to an insertion. */
			void drawInsert(size_t count = 1);

			void printGraphemes(String);

			virtual bool canDraw() const override;

			/** Focuses the TextInput and jumps to its cursor. */
			void focus() override;

			/** Moves the terminal cursor to the position of the TextInput cursor. */
			void jumpCursor();

			/** Jumps to the cursor. */
			virtual void jumpFocus() override;

			/** Moves the terminal cursor to the position of the TextInput cursor if the TextInput is focused. Returns
			 *  true if the TextInput is focused and the cursor was moved. */
			bool tryJump();

			virtual Terminal * getTerminal() override { return terminal; }
			virtual Container * getParent() const override { return parent; }

			/** Writes the contents of the buffer to an output stream. */
			friend std::ostream & operator<<(std::ostream &os, const TextInput &input);
	};
}

#endif
