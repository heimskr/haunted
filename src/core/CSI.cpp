#include <stdexcept>

#include "haunted/core/CSI.h"
#include "haunted/core/Terminal.h"
#include "haunted/core/Util.h"

namespace Haunted {
	void CSI::scanNumber(unsigned int &target, ssize_t &i, const std::string &str) {
		for (ssize_t p = 1; 0 <= i && Util::isNumeric(str[i]); --i) {
			target += p * (str[i] - '0');
			p *= 10;
		}
	}

	void CSI::parseU(const std::string &str) {
		// Format for CSI u: "CSI [number];[modifier] u"

		first = second = 0;
		const ssize_t len = str.size();
		ssize_t i = len - 2;
		suffix = str[len - 1];

		if (!Util::isNumeric(str[0]))
			throw std::invalid_argument("CSI u: first character isn't numeric");

		if (str[len - 3] != ';')
			throw std::invalid_argument("CSI u: semicolon not in expected position");

		const char penult = str[len - 2];
		if (!Util::inRange(penult, '1', '8'))
			throw std::invalid_argument("CSI u: invalid penultimate character: '" + std::string(1, penult) + "'");

		second = penult - '0';

		scanNumber(first, i = len - 4, str);
		
		if (i != -1)
			throw std::invalid_argument("CSI u: parsing failed for \"" + str + "\"");
	}

	void CSI::parseSpecial(const std::string &str) {
		// Format for CSI ~ (special): "CSI [number];[modifier] ~" or "CSI [number] ~"

		first = second = 0;
		const ssize_t len = str.size();
		ssize_t i = len - 2;
		suffix = str[len - 1];

		// Although it can have either one or two components, the first character is always a number.
		if (!Util::isNumeric(str[0]))
			throw std::invalid_argument("CSI ~: first character isn't numeric");

		ssize_t semicolon_pos = str.find(';');

		if (static_cast<size_t>(semicolon_pos) == std::string::npos) {
			// If there's no semicolon, it's just one component, so we just scan from the end of the string and
			// assume everything's numeric. The modifier is 1 (none) by default.
			second = 1;
			scanNumber(first, i, str);
		} else if (semicolon_pos == len - 2) {
			// If the second-to-last character is a semicolon, then there's nothing between the semicolon and the
			// final character. That's invalid.
			throw std::invalid_argument("CSI ~: missing number after semicolon");

		} else if (!Util::inRange(str[len - 2], '1', '8')) {
			// If the semicolon isn't incorrect, then the character after it has to represent a valid modifier.
			throw std::invalid_argument("CSI ~: invalid character after semicolon");

		} else {
			// If the semicolon and modifier are valid, take the modifier and scan the string starting right before
			// the semicolon.
			second = str[len - 2] - '0';
			scanNumber(first, i = semicolon_pos - 1, str);
		}

		// The scan needs to end at the beginning of the string; otherwise, the first number is invalid.
		if (i != -1)
			throw std::invalid_argument("CSI ~: parsing failed for \"" + str + "\" at " + std::to_string(i));
	}

	CSI::CSI(int first, int second, char suffix): first(first), second(second), suffix(suffix) {
		switch (suffix) {
			case 'u': type = CSIType::U; break;
			case '~': type = CSIType::Special; break;
			default: type = CSIType::ReallySpecial;
		}
	}

	Key CSI::getKey() const {
		switch (suffix) {
			case 'A': return KeyType::UpArrow;
			case 'B': return KeyType::DownArrow;
			case 'C': return KeyType::RightArrow;
			case 'D': return KeyType::LeftArrow;
			case 'F': return KeyType::End;
			case 'H': return KeyType::Home;
			case 'P': return KeyType::F1;
			case 'Q': return KeyType::F2;
			case 'R': return KeyType::F3;
			case 'S': return KeyType::F4;
			case 'Z': return Key(KeyType::Tab).shift();
			case 'u': return KeyType(first);
			case '~':
				switch (first) {
					case 2:  return KeyType::Insert;
					case 3:  return KeyType::Del;
					case 147:
					case 5:  return KeyType::PageUp;
					case 148:
					case 6:  return KeyType::PageDown;
					case 1:
					case 7:  return KeyType::Home;
					case 4:
					case 8:  return KeyType::End;
					case 11: return KeyType::F1;
					case 12: return KeyType::F2;
					case 13: return KeyType::F3;
					case 14: return KeyType::F4;
					case 15: return KeyType::F5;
					case 17: return KeyType::F6;
					case 18: return KeyType::F7;
					case 19: return KeyType::F8;
					case 20: return KeyType::F9;
					case 21: return KeyType::F10;
					case 23: return KeyType::F11;
					case 24: return KeyType::F12;
					default: throw std::invalid_argument("Unexpected special key: " + std::to_string(first));
				}
			default: throw std::invalid_argument("Unexpected suffix: '" + std::string(1, suffix) + "'");
		}
	}
	
	CSI::CSI(const std::string &str): first(0), second(0) {
		static const std::string endings = "u~ABCDFHPQRSZ";
		size_t len = str.length();

		// If the string is empty, we need to give up immediately.
		if (len == 0)
			throw std::invalid_argument("CSI string is empty");

		suffix = str[len - 1];

		// There's a specific set of characters that can serve as the final character in a CSI sequence. If the last
		// character of this string isn't among them, it's invalid.
		if (endings.find(suffix) == std::string::npos)
			throw std::invalid_argument("Invalid CSI ending: '" + std::string(1, suffix) + "'");
		
		if (suffix == 'u') {
			parseU(str);
		} else if (suffix == '~') {
			parseSpecial(str);
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
			if (str[0] != '1' || str[1] != ';' || !Util::inRange(str[2], '1', '8'))
				throw std::invalid_argument("Parsing failed for \"really special\" CSI: \"" + str + "\"");

			first = str[0] - '0';
			second = str[2] - '0';
		}
	}

	bool CSI::isCSIu(const std::string &str) {
		const size_t len = str.size();

		// There are four components in a CSI u sequence, so the string has to be at least 4 bytes.
		// It also needs to start with a number.
		if (len < 4 || !Util::isNumeric(str[0]))
			return false;

		size_t i = 0;
		for (; i < len && Util::isNumeric(str[i]); ++i);
		if (str[i++] != ';') return false;
		for (; i < len && Util::isNumeric(str[i]); ++i);
		return i == len - 1 && str[i] == 'u';
	}

	CSI::operator std::pair<int, int>() const {
		return {first, second};
	}
}
