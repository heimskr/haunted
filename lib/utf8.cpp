#include "utf8.h"
#include "unicode/unistr.h"

#include <iostream>
#include <sstream>
#include "core/defs.h"

namespace haunted {
	using utf8chars = std::vector<utf8char>;

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

	utf8char::utf8char(const char *str, size_t len, size_t offset): cp(0) {
		for (size_t i = 0; i < len && i < 4; ++i)
			cp |= static_cast<unsigned char>(str[offset + i]) << (8 * i);
	}

	utf8char::operator int32_t() const {
		return cp;
	}

	utf8char::operator std::string() const {
		const char chars[5] = {
			static_cast<char>(cp & 0xff),
			static_cast<char>((cp >> 8) & 0xff),
			static_cast<char>((cp >> 16) & 0xff),
			static_cast<char>((cp >> 24) & 0xff),
			0
		};

		return chars;
	}

	std::vector<utf8char> utf8str::decode_utf8(const std::string &str) {
		std::vector<utf8char> out {};
		size_t i = 0;
		const char *cstr = str.c_str();
		size_t len = str.length();
		std::cout << "length = " << len << std::endl;
		while (i < len) {
			char c = str[i];
			size_t width = utf8::width(c);
			out.push_back({cstr, width, i});

			i += width;
		}
	}

	utf8char utf8str::operator[](ssize_t index) const {
		return unistr[index];
	}

	std::ostream & operator<<(std::ostream &os, const utf8char &input) {
		os << std::string(input);
		return os;
	}

	std::string operator+(const std::string &left, const utf8char &right) {
		return left + std::string(right);
	}

	std::string operator+(const char *left, const utf8char &right) {
		return std::string(left) + std::string(right);
	}

	utf8str & utf8str::operator=(const utf8str &rhs) {
		clear();
		unistr = rhs.unistr;
		return *this;
	}

	utf8str & utf8str::operator=(const utf8chars &rhs) {
		clear();
		unistr = rhs;
		return *this;
	}

	utf8str & utf8str::operator=(const std::string &rhs) {
		clear();
		unistr = decode_utf8(rhs);
		return *this;
	}

	utf8str & utf8str::operator=(utf8str &&rhs) {
		clear();
		unistr = std::move(rhs.unistr);
		return *this;
	}

	utf8str & utf8str::operator=(utf8chars &&rhs) {
		clear();
		unistr = std::move(rhs);
		return *this;
	}

	utf8str & utf8str::operator=(std::string &&rhs) {
		clear();
		unistr = decode_utf8(std::move(rhs));
		return *this;
	}

	bool utf8str::operator==(const utf8str &rhs) const {
		return unistr == rhs.unistr;
	}

	utf8str::operator std::string() const {return "???";
		// std::string out;
		// return unistr.toUTF8String(out);
	}

	void utf8str::append(utf8str &ustr) {
		unistr.insert(unistr.end() - 1, ustr.begin(), ustr.end());
	}

	void utf8str::append(const std::string &str) {
		unistr.push_back(str);
	}

	void utf8str::append(char ch) {
		unistr.push_back(ch);
	}

	std::string utf8str::substr(ssize_t start, ssize_t length) const {return "";
		// std::string out;
		// return icu::UnicodeString(unistr, start, length).toUTF8String(out);
	}

	std::string utf8str::substr(ssize_t start) const {return "";
		// std::string out;
		// return icu::UnicodeString(unistr, start).toUTF8String(out);
	}

	void utf8str::insert(ssize_t pos, const std::string &str) {
		// auto ustr = icu::UnicodeString::fromUTF8(str);
		// std::stringstream ss("");
		// ss << std::hex;
		// for (char ch: str) ss << static_cast<int>(ch) << " ";
		// DBG("Length of UnicodeString ( " << ss.str() << "): " << ustr.countChar32());
		
		// unistr.insert(pos, icu::UnicodeString::fromUTF8(str).char32At(0));
		unistr.insert(unistr.begin() + pos, str);
	}

	void utf8str::insert(ssize_t pos, char ch) {
		// insert(pos, std::string(1, ch));
		unistr.insert(unistr.begin() + pos, ch);
	}

	void utf8str::erase(ssize_t start, ssize_t length) {
		unistr.erase(unistr.begin() + start, unistr.begin() + start + length);
	}

	void utf8str::erase(ssize_t start) {
		unistr.erase(unistr.begin() + start, unistr.end());
	}

	size_t utf8str::size()   const { return unistr.size();  }
	size_t utf8str::length() const { return size();  }
	bool   utf8str::empty()  const { return size() == 0; }
	void   utf8str::clear()        { unistr.clear(); }
	utf8chars::iterator utf8str::begin() { return unistr.begin(); }
	utf8chars::iterator utf8str::end()   { return unistr.end();   }

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
