#include "haunted/ui/simpleline.h"
#include "lib/formicine/performance.h"

namespace haunted::ui {
	simpleline::simpleline(const std::string &text_, int continuation_): continuation(continuation_), text(text_) {
		text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
		text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
	}

	simpleline::operator std::string() {
		auto w = formicine::perf.watch("simpleline::operator std::string()");
		return text;
	}

	bool simpleline::operator==(simpleline &other) {
		return get_continuation() == other.get_continuation() && text == other.text;
	}
}

std::ostream & operator<<(std::ostream &os, const haunted::ui::simpleline &line) {
	auto w = formicine::perf.watch("operator<<(std::ostream, simpleline)");
	os << line.text;
	return os;
}
