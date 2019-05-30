#include "lib/utf8.h"
#include "unicode/unistr.h"

namespace spjalla {
	/**
	 * Determines the expected codepoint width for a given start byte.
	 * @param ch The initial byte of the codepoint.
	 * @return The expected width in bytes if the byte is a valid starting byte; 0 otherwise.
	 */
	size_t utf8::width(unsigned char uch) {
		if (uch < 0x80)           return 1;
		if ((uch & 0xe0) == 0xc0) return 2;
		if ((uch & 0xf0) == 0xe0) return 3;
		if ((uch & 0xf8) == 0xf0) return 4;
		return 0;
	}

	utf8char utf8str::operator[](ssize_t index) const {
		return unistr.char32At(index);
	}

	utf8str & utf8str::operator=(const utf8str &rhs) {
		clear();
		unistr = rhs.unistr;
		return *this;
	}

	utf8str & utf8str::operator=(const icu::UnicodeString &rhs) {
		clear();
		unistr = rhs;
		return *this;
	}

	utf8str & utf8str::operator=(utf8str &&rhs) {
		clear();
		unistr = std::move(rhs.unistr);
		return *this;
	}

	utf8str & utf8str::operator=(icu::UnicodeString &&rhs) {
		clear();
		unistr = std::move(rhs);
		return *this;
	}

	bool utf8str::operator==(const utf8str &rhs) const {
		return unistr == rhs.unistr;
	}

	utf8str::operator std::string() const {
		std::string out;
		return unistr.toUTF8String(out);
	}

	void utf8str::append(const utf8str &ustr) {
		unistr.append(ustr.unistr);
	}

	void utf8str::append(const std::string &str) {
		unistr.append(icu::UnicodeString::fromUTF8(str));
	}

	void utf8str::append(char ch) {
		unistr.append(static_cast<char16_t>(ch));
	}

	std::string utf8str::substr(ssize_t start, ssize_t length) const {
		std::string out;
		return icu::UnicodeString(unistr, start, length).toUTF8String(out);
	}

	std::string utf8str::substr(ssize_t start) const {
		std::string out;
		return icu::UnicodeString(unistr, start).toUTF8String(out);
	}

	void utf8str::insert(ssize_t pos, const std::string &str) {
		unistr.insert(pos, icu::UnicodeString::fromUTF8(str));
	}

	void utf8str::insert(ssize_t pos, char ch) {
		insert(pos, std::string(1, ch));
	}

	void utf8str::erase(ssize_t start, ssize_t length) {
		unistr.remove(start, length);
	}

	void utf8str::erase(ssize_t start) {
		unistr.remove(start);
	}

	size_t utf8str::size()   const { return unistr.length();  }
	size_t utf8str::length() const { return unistr.length();  }
	bool   utf8str::empty()  const { return unistr.isEmpty(); }
	void   utf8str::clear()        { unistr.remove(); }

	std::ostream & operator<<(std::ostream &os, const utf8str &input) {
		os << std::string(input);
		return os;
	}

	std::string operator+(const std::string &left, const utf8str &right) {
		return left + std::string(right);
	}

	std::string operator+(const char *left, const utf8str &right) {
		return std::string(left) + std::string(right);
	}
}
