#include <stdexcept>
#include <string>
#include <vector>

#include "haunted/core/Mouse.h"

#include "lib/formicine/futil.h"

namespace Haunted {
	MouseReport::MouseReport(long type, char final_char, long x_, long y_): finalChar(final_char), x(x_), y(y_) {
		decodeType(type, final_char, action, button, mods);
	}

	MouseReport::MouseReport(const std::string &combined): finalChar(combined.back()) {
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
		decodeType(type, combined.back(), action, button, mods);
	}

	std::string MouseReport::str() const {
		std::string action_str;
		switch (action) {
			case MouseAction::Down: action_str = "down"; break;
			case MouseAction::Drag: action_str = "drag"; break;
			case MouseAction::Move: action_str = "move"; break;
			case MouseAction::Up:   action_str = "up";   break;
			case MouseAction::ScrollDown: action_str = "scrolldown"; break;
			case MouseAction::ScrollUp:   action_str = "scrollup";   break;
			default: action_str = "invalid";
		}

		const std::string out = Key::modString(mods) + " " + action_str + " " + std::to_string(x) + " "
			+ std::to_string(y);
		if (action != MouseAction::Move) // TODO: support middle mouse button
			return out + (button == MouseButton::Left? " left" : " right");
		return out;
	}

	void MouseReport::decodeType(long num, char fchar, MouseAction &action, MouseButton &button, ModSet &mods) {
		button = num & 2? MouseButton::Right : MouseButton::Left;

		mods = {};
		if (num &  4) mods.set(0); // shift
		if (num &  8) mods.set(1); // meta
		if (num & 16) mods.set(2); // ctrl

		if (num & 0x40) { // 64: scroll
			action = num & 1? MouseAction::ScrollDown : MouseAction::ScrollUp;
		} else if (num & 0x20) { // 32: move
			action = MouseAction::Move;
			button = num & 1? MouseButton::Right : MouseButton::Left;
		} else if (fchar == 'M') {
			action = MouseAction::Down;
		} else if (fchar == 'm') {
			action = MouseAction::Up;
		} else {
			DBG("Invalid final character: '" << fchar << "'");
			throw std::invalid_argument("Invalid final character");
		}
	}
}
