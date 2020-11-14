#ifndef HAUNTED_UI_SIMPLELINE_H_
#define HAUNTED_UI_SIMPLELINE_H_

#include "haunted/ui/TextLine.h"

namespace Haunted::UI {
	/**
	 * Represents a line of text with static, unchanging content.
	 */
	template <template <typename... T> typename C = std::vector>
	struct SimpleLine: public TextLine<C> {
		int continuation = 0;

		/** The raw text of the line. */
		std::string text = "";

		SimpleLine(const std::string &text_, int continuation_ = 0): continuation(continuation_), text(text_) {
			text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
			text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
		}

		SimpleLine(int continuation_): continuation(continuation_) {}

		SimpleLine(): SimpleLine("", 0) {}

		virtual operator std::string() override {
			auto w = formicine::perf.watch("SimpleLine::operator std::string()");
			return text;
		}

		int getContinuation() override { return continuation; }

		virtual bool operator==(SimpleLine &other) {
			return getContinuation() == other.getContinuation() && text == other.text;
		}
	};

	template <template <typename... T> typename C>
	std::ostream & operator<<(std::ostream &os, const Haunted::UI::SimpleLine<C> &line) {
		auto w = formicine::perf.watch("operator<<(std::ostream, SimpleLine)");
		return os << line.text;
	}
}

#endif
