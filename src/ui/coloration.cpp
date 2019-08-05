#include "core/defs.h"
#include "ui/coloration.h"

namespace haunted::ui {
	bool coloration::set_foreground(ansi::color foreground) {
		if (foreground == last_foreground)
			return false;
		
		auto lock = get_lock();
		DBG("Switching foreground: " << ansi::get_name(last_foreground) << " → "_d << ansi::get_name(foreground));
		*out_stream << ansi::get_fg(last_foreground = foreground);
		return true;
	}

	bool coloration::set_background(ansi::color background) {
		if (background == last_background)
			return false;
		
		auto lock = get_lock();
		DBG("Switching background: " << ansi::get_name(last_background) << " → "_d << ansi::get_name(background));
		*out_stream << ansi::get_bg(last_background = background);
		return true;
	}

	bool coloration::set_both(ansi::color foreground, ansi::color background) {
		bool fg = set_foreground(foreground), bg = set_background(background);
		return fg || bg;
	}

	void coloration::apply() {
		*out_stream << ansi::get_fg(last_foreground) << ansi::get_bg(last_background);
	}

	bool coloration::reset() {
		DBG("Resetting colors.");
		return set_both(ansi::color::normal, ansi::color::normal);
	}

	void coloration::debug() {
		DBG("Foreground: " << ansi::get_name(last_foreground) << ", background: " << ansi::get_name(last_background));
	}
}
