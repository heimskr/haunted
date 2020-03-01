#ifndef HAUNTED_UI_SIMPLELINE_H_
#define HAUNTED_UI_SIMPLELINE_H_

#include "haunted/ui/textline.h"

namespace Haunted::UI {
	/**
	 * Represents a line of text with static, unchanging content.
	 */
	struct simpleline: public textline {
		int continuation = 0;

		/** The raw text of the line. */
		std::string text = "";

		simpleline(const std::string &text_, int continuation_ = 0);
		simpleline(int continuation_): continuation(continuation_) {}
		simpleline(): simpleline("", 0) {}

		virtual operator std::string() override;

		int get_continuation() override { return continuation; }

		virtual bool operator==(simpleline &);
	};
}

#endif
