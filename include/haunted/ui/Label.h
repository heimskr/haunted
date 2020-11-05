#ifndef HAUNTED_UI_LABEL_H_
#define HAUNTED_UI_LABEL_H_

#include <cstddef>
#include <functional>
#include <ostream>
#include <string>
#include <unordered_set>

#include "haunted/core/Defs.h"
#include "haunted/core/Key.h"
#include "haunted/ui/Container.h"
#include "haunted/ui/Control.h"
#include "haunted/ui/Colored.h"

#include "lib/UTF8.h"

namespace Haunted::UI {
	/**
	 * Represents a single-row control that displays text.
	 */
	class Label: public Control, public Colored {
		private:
			/** Contains the label's text. */
			std::string text;

			/** If the label's text is too long to be rendered, the text will be cut off and the cutoff string will be
			 *  appended. */
			std::string cutoff;

			/** Whether to request a resize from the parent whenever the label text changes. */
			bool autoresize = false;

		public:
			/** Constructs a textinput with a parent, a position, text, an autoresize flag and a cutoff string. */
			Label(Container *parent, const Position &pos = {}, const std::string &text = "", bool autoresize = false,
				const std::string &cutoff = "...");

			Label(const std::string &text, bool autoresize = false, const std::string &cutoff = "..."):
				Label(nullptr, {}, text, autoresize, cutoff) {}

			/** Erases the label text. */
			void clear();

			/** Returns the label text. */
			std::string getText() const { return text; }

			/** Sets the label text. */
			void setText(const std::string &);

			bool getAutoresize() const { return autoresize; }
			void setAutoresize(bool);

			/** Returns the number of characters in the label. */
			size_t length() const { return text.length(); }
			size_t size()   const { return text.length(); }

			/** Renders the control onto the terminal. */
			virtual void draw() override;

			virtual bool canDraw() const override;

			/** Returns the contents of the buffer. */
			operator std::string() const { return text; }

			virtual Terminal * getTerminal() override { return terminal; }
			virtual Container * getParent() const override { return parent; }

			/** Writes the label text to an output stream. */
			friend std::ostream & operator<<(std::ostream &, const Label &);
	};
}

#endif
