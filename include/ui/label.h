#ifndef HAUNTED_UI_LABEL_H_
#define HAUNTED_UI_LABEL_H_

#include <cstddef>
#include <functional>
#include <ostream>
#include <string>
#include <unordered_set>

#include "../core/defs.h"
#include "../core/key.h"
#include "../lib/utf8.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted::ui {
	/**
	 * Represents a single-row control that displays text.
	 */
	class label: public virtual control {
		private:
			/** Contains the label's text. */
			std::string text;

			/** If the label's text is too long to be rendered, the text will be cut off and the cutoff string will be
			 *  appended. */
			std::string cutoff;

			/** Whether to request a resize from the parent whenever the label text changes. */
			bool autoresize;
		public:
			/** Constructs a textinput with a parent, a position, text, an autoresize flag and a cutoff string. */
			label(container *parent, position pos, const std::string &text, bool autoresize = false,
				const std::string &cutoff = "...");

			label(const std::string &text, bool autoresize = false, const std::string &cutoff = "..."):
				label(nullptr, {}, text, autoresize, cutoff) {}

			/** Erases the contents of the buffer and resets the cursor. */
			void clear() { text.clear(); }

			/** Returns the label text. */
			std::string get_text() const { return text; }

			/** Sets the label text. */
			void set_text(const std::string &);

			bool get_autoresize() const { return autoresize; }
			void set_autoresize(bool);

			/** Returns the number of characters in the label. */
			size_t length() const { return text.length(); }
			size_t size()   const { return text.length(); }

			/** Renders the control onto the terminal. */
			virtual void draw() override;

			virtual bool can_draw() const override;

			/** Returns the contents of the buffer. */
			operator std::string() const { return text; }

			/** Writes the label text to an output stream. */
			friend std::ostream & operator<<(std::ostream &os, const label &input);
	};
}

#endif
