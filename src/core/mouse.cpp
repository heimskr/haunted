#include <stdexcept>
#include <string>
#include <vector>

#include "haunted/core/mouse.h"

#include "lib/formicine/futil.h"

namespace Haunted {
	mouse_report::mouse_report(long type, char final_char_, long x_, long y_): final_char(final_char_), x(x_), y(y_) {
		decode_type(type, final_char_, action, button, mods);
	}

	mouse_report::mouse_report(const std::string &combined): final_char(combined.back()) {
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
		--x;
		--y;
		decode_type(type, combined.back(), action, button, mods);
	}

	std::string mouse_report::str() const {
		std::string action_str;
		switch (action) {
			case mouse_action::down: action_str = "down"; break;
			case mouse_action::drag: action_str = "drag"; break;
			case mouse_action::move: action_str = "move"; break;
			case mouse_action::up:   action_str = "up";   break;
			case mouse_action::scrolldown: action_str = "scrolldown"; break;
			case mouse_action::scrollup:   action_str = "scrollup";   break;
			default: action_str = "invalid";
		}

		const std::string out = key::mod_str(mods) + " " + action_str + " " + std::to_string(x) + " "
			+ std::to_string(y);
		if (action != mouse_action::move) // TODO: support middle mouse button
			return out + (button == mouse_button::left? " left" : " right");
		return out;
	}

	void mouse_report::decode_type(long num, char fchar, mouse_action &action, mouse_button &button, modset &mods) {
		button = num & 2? mouse_button::right : mouse_button::left;

		mods = {};
		if (num &  4) mods.set(0); // shift
		if (num &  8) mods.set(1); // meta
		if (num & 16) mods.set(2); // ctrl

		if (num & 0x40) { // 64: scroll
			action = num & 1? mouse_action::scrolldown : mouse_action::scrollup;
		} else if (num & 0x20) { // 32: move
			action = mouse_action::move;
			button = num & 1? mouse_button::right : mouse_button::left;
		} else if (fchar == 'M') {
			action = mouse_action::down;
		} else if (fchar == 'm') {
			action = mouse_action::up;
		} else {
			DBG("Invalid final character: '" << fchar << "'");
			throw std::invalid_argument("Invalid final character");
		}
	}
}
