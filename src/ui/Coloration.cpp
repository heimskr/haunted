#include "haunted/core/Defs.h"
#include "haunted/ui/Coloration.h"

namespace Haunted::UI {
	bool Coloration::setForeground(ansi::color foreground) {
		if (foreground == lastForeground)
			return false;

		auto lock = getLock();
		*outStream << ansi::get_fg(lastForeground = foreground);
		return true;
	}

	bool Coloration::setBackground(ansi::color background) {
		if (background == lastBackground)
			return false;

		auto lock = getLock();
		*outStream << ansi::get_bg(lastBackground = background);
		return true;
	}

	bool Coloration::setBoth(ansi::color foreground, ansi::color background) {
		bool fg = setForeground(foreground), bg = setBackground(background);
		return fg || bg;
	}

	void Coloration::apply() {
		*outStream << ansi::get_fg(lastForeground) << ansi::get_bg(lastBackground);
	}

	bool Coloration::reset() {
		return setBoth(ansi::color::normal, ansi::color::normal);
	}

	void Coloration::debug() {
		DBG("Foreground: " << ansi::get_name(lastForeground) << ", background: " << ansi::get_name(lastBackground));
	}
}
