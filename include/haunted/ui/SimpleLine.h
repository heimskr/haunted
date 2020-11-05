#ifndef HAUNTED_UI_SIMPLELINE_H_
#define HAUNTED_UI_SIMPLELINE_H_

#include "haunted/ui/TextLine.h"

namespace Haunted::UI {
	/**
	 * Represents a line of text with static, unchanging content.
	 */
	struct SimpleLine: public TextLine {
		int continuation = 0;

		/** The raw text of the line. */
		std::string text = "";

		SimpleLine(const std::string &text_, int continuation_ = 0);
		SimpleLine(int continuation_): continuation(continuation_) {}
		SimpleLine(): SimpleLine("", 0) {}

		virtual operator std::string() override;

		int getContinuation() override { return continuation; }

		virtual bool operator==(SimpleLine &);
	};
}

#endif
