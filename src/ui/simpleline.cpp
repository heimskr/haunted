#include "haunted/ui/SimpleLine.h"
#include "lib/formicine/performance.h"

namespace Haunted::UI {
	SimpleLine::SimpleLine(const std::string &text_, int continuation_): continuation(continuation_), text(text_) {
		text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
		text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
	}

	SimpleLine::operator std::string() {
		auto w = formicine::perf.watch("SimpleLine::operator std::string()");
		return text;
	}

	bool SimpleLine::operator==(SimpleLine &other) {
		return getContinuation() == other.getContinuation() && text == other.text;
	}
}

std::ostream & operator<<(std::ostream &os, const Haunted::UI::SimpleLine &line) {
	auto w = formicine::perf.watch("operator<<(std::ostream, SimpleLine)");
	os << line.text;
	return os;
}
