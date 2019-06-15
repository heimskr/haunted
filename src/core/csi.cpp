#include <stdexcept>

#include "core/csi.h"
#include "core/terminal.h"
#include "core/util.h"

namespace haunted {
	void csi::scan_number(int &target, ssize_t &i, const std::string &str) {
		for (ssize_t p = 1; 0 <= i && util::is_numeric(str[i]); --i) {
			target += p * (str[i] - '0');
			p *= 10;
		}
	}

	void csi::parse_u(const std::string &str) {
		// Format for CSI u: "CSI [number];[modifier] u"

		first = second = suffix = 0;
		const ssize_t len = str.size();
		ssize_t i = len - 2;
		suffix = str[len - 1];

		if (!util::is_numeric(str[0]))
			throw std::invalid_argument("CSI u: first character isn't numeric");

		if (str[len - 3] != ';')
			throw std::invalid_argument("CSI u: semicolon not in expected position");

		const char penult = str[len - 2];
		if (!util::in_range(penult, '1', '8'))
			throw std::invalid_argument("CSI u: invalid penultimate character: '" + std::string(penult, 1) + "'");

		second = penult - '0';

		scan_number(first, i = len - 4, str);
		
		if (i != -1)
			throw std::invalid_argument("CSI u: parsing failed for \"" + str + "\"");
	}

	void csi::parse_special(const std::string &str) {
		// Format for CSI ~ (special): "CSI [number];[modifier] ~" or "CSI [number] ~"
		// Prior to commit 246667c, the number/modifier is backwards in iTerm.

		first = second = suffix = 0;
		const ssize_t len = str.size();
		ssize_t i = len - 2;
		suffix = str[len - 1];

		// Although it can have either one or two components, the first character is always a number.
		if (!util::is_numeric(str[0]))
			throw std::invalid_argument("CSI ~: first character isn't numeric");

		ssize_t semicolon_pos = str.find(';');

		if (static_cast<size_t>(semicolon_pos) == std::string::npos) {
			// If there's no semicolon, it's just one component, so we just scan from the end of the string and
			// assume everything's numeric. The modifier is 1 (none) by default.
			second = 1;
			scan_number(first, i, str);

#ifdef ITERM_HACK
		} else if (terminal::is_iterm()) {
			// Because the current (beta) version of iTerm incorrectly orders the modifier before the keycode as of June
			// 15, 2019, we need to accommodate the bug by parsing the numbers backwards.
			if (semicolon_pos != 1)
				throw std::invalid_argument("CSI ~ (backwards): invalid semicolon position");

			second = str[0] - '0';
			scan_number(first, i, str);
			if (i != semicolon_pos)
				throw std::invalid_argument("CSI ~ (backwards): parsing failed for \"" + str + "\"");

			return;
#endif

		} else if (semicolon_pos == len - 2) {
			// If the second-to-last character is a semicolon, then there's nothing between the semicolon and the
			// final character. That's invalid.
			throw std::invalid_argument("CSI ~: missing number after semicolon");

		} else if (!util::in_range(str[len - 2], '1', '8')) {
			// If the semicolon isn't incorrect, then the character after it has to represent a valid modifier.
			throw std::invalid_argument("CSI ~: invalid character after semicolon");

		} else {
			// If the semicolon and modifier are valid, take the modifier and scan the string starting right before
			// the semicolon.
			second = str[len - 2] - '0';
			scan_number(first, i = semicolon_pos - 1, str);
		}

		// The scan needs to end at the beginning of the string; otherwise, the first number is invalid.
		if (i != -1)
			throw std::invalid_argument("CSI ~: parsing failed for \"" + str + "\" at " + std::to_string(i));
	}

	csi::csi(int first, int second, char suffix): first(first), second(second), suffix(suffix) {
		switch (suffix) {
			case 'u': type = csi_type::u;
			case '~': type = csi_type::special;
			default: type = csi_type::really_special;
		}
	}

	ktype csi::get_key() const {
		switch (suffix) {
			case 'A': return ktype::up_arrow;
			case 'B': return ktype::down_arrow;
			case 'C': return ktype::right_arrow;
			case 'D': return ktype::left_arrow;
			case 'F': return ktype::end;
			case 'H': return ktype::home;
			case 'P': return ktype::f1;
			case 'Q': return ktype::f2;
			case 'R': return ktype::f3;
			case 'S': return ktype::f4;
			case 'u': return ktype(first);
			case '~':
				switch (first) {
					case 2:  return ktype::insert;
					case 3:  return ktype::del;
					case 5:  return ktype::page_up;
					case 6:  return ktype::page_down;
					case 7:  return ktype::home;
					case 8:  return ktype::end;
					case 11: return ktype::f1;
					case 12: return ktype::f2;
					case 13: return ktype::f3;
					case 14: return ktype::f4;
					case 15: return ktype::f5;
					case 17: return ktype::f6;
					case 18: return ktype::f7;
					case 19: return ktype::f8;
					case 20: return ktype::f9;
					case 21: return ktype::f10;
					case 23: return ktype::f11;
					case 24: return ktype::f12;
					default: throw std::invalid_argument("Unexpected special key: " + std::to_string(first));
				}
			default: throw std::invalid_argument("Unexpected suffix: '" + std::string(suffix, 1) + "'");
		}
	}
	
	csi::csi(const std::string &str): first(0), second(0) {
		static const std::string endings = "u~ABCDFHPQRS";
		size_t len = str.length();

		// If the string is empty, we need to give up immediately.
		if (len == 0)
			throw std::invalid_argument("CSI string is empty");

		suffix = str[len - 1];

		// There's a specific set of characters that can serve as the final character in a CSI sequence. If the last
		// character of this string isn't among them, it's invalid.
		if (endings.find(suffix) == std::string::npos)
			throw std::invalid_argument("Invalid CSI ending: '" + std::string(suffix, 1) + "'");
		
		if (suffix == 'u') {
			parse_u(str);
		} else if (suffix == '~') {
			parse_special(str);
		} else {
			// At this point, the sequence must be a "really special" type. That means it's either
			// "CSI 1;[modifier] {ABCDFHPQRS}" or "CSI {ABCDFHPQRS}".
			// The length (which includes the suffix) has to be either 1 or 4.
			if (len == 1) {
				// If the length is 1, then the CSI consists entirely of the suffix and
				// the keycode and modifier are both implicitly 1.
				first = second = 1;
				return;
			} else if (len != 4) {
				throw std::invalid_argument("Invalid length for \"really special\" CSI: " + std::to_string(len));
			}

			// This part is really easy; we know that the part before the suffix is exactly three characters long, that
			// the first two characters are "1;" and that the third character is a number between 1 and 8 (inclusive).
			if (str[0] != '1' || str[1] != ';' || !util::in_range(str[2], '1', '8'))
				throw std::invalid_argument("Parsing failed for \"really special\" CSI: \"" + str + "\"");
		}
	}

	bool csi::is_csiu(const std::string &str) {
		const size_t len = str.size();

		// There are four components in a CSI u sequence, so the string has to be at least 4 bytes.
		// It also needs to start with a number.
		if (len < 4 || !util::is_numeric(str[0]))
			return false;

		size_t i = 0;
		for (; i < len && util::is_numeric(str[i]); ++i);
		if (str[i++] != ';') return false;
		for (; i < len && util::is_numeric(str[i]); ++i);
		return i == len - 1 && str[i] == 'u';
	}
}
