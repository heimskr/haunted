#include <stdexcept>
#include <string>
#include <vector>

#include "haunted/core/mouse.h"

#include "lib/formicine/futil.h"

namespace haunted {
	mouse_report::mouse_report(long type, long x_, long y_): x(x_), y(y_) {
		DBG("mouse_report::mouse_report(type=[" << type << "]");
	}

	mouse_report::mouse_report(const std::string &combined) {
		std::vector<std::string> numbers = formicine::util::split(combined.substr(1, combined.length() - 2), ";");

		if (numbers.size() != 3)
			throw std::invalid_argument("Invalid number of parameters in mouse report");
		
		long type;
		if (!formicine::util::parse_long(numbers[0], type))
			throw std::invalid_argument("Couldn't parse mouse report type as a number");
		if (!formicine::util::parse_long(numbers[1], x))
			throw std::invalid_argument("Couldn't parse mouse report x coordinate as a number");
		if (!formicine::util::parse_long(numbers[2], y))
			throw std::invalid_argument("Couldn't parse mouse report y coordinate as a number");
		decode_type(type, combined.back(), action, button, mods);
	}

	void mouse_report::decode_type(long num, char fchar, mouse_action &action, mouse_button &button, modset &mods) {
		// 32 M: drag | left  | none
		// 35 M: move | left  | none
		//  0 M: down | left  | none
		//  0 m: up   | left  | none
		//  4 M: down | left  | shift
		//  8 M: down | left  | meta
		// 12 M: down | left  | shift+meta
		// 16 M: down | left  | ctrl
		//  2 M: down | right | none
		// 10 M: down | right | meta
		// 14 M: down | right | shift+meta
		// 18 M: down | right | shift
		std::string d {};

		button = num & 2? mouse_button::right : mouse_button::left;
		d += num & 2? "right" : "left";

		mods = {};
		if (num &  4) { mods.set(0); d += ", shift"; } // shift
		if (num &  8) { mods.set(1); d += ", meta"; } // meta
		if (num & 16) { mods.set(2); d += ", ctrl"; } // ctrl

		if (num & 0x40) { // 64: scroll
			action = num & 1? mouse_action::scrolldown : mouse_action::scrollup;
			d += ", scroll " + std::string(num & 1? "down" : "up");
		} else if (num & 0x20) { // 32: move
			action = mouse_action::move;
			button = num & 1? mouse_button::right : mouse_button::left;
			d += ", move";
		} else if (fchar == 'M') {
			action = mouse_action::down;
			d += ", down";
		} else if (fchar == 'm') {
			action = mouse_action::up;
			d += ", up";
		} else {
			DBG("Invalid final character: '" << fchar << "'");
			throw std::invalid_argument("Invalid final character");
		}

		DBG(d << ": " << num);
	}
}
