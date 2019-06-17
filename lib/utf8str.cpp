#include <iostream>
#include <sstream>

#include "utf8.h"

namespace haunted {
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

		return out;
	}

	utf8str::utf8str(const std::string &str) {
		unistr = utf8str::decode_utf8(str);
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

	// std::string utf8str::substr(ssize_t start, ssize_t length) const {
	std::string utf8str::substr(ssize_t, ssize_t) const {return "@";
		// std::string out;
		// return icu::UnicodeString(unistr, start, length).toUTF8String(out);
	}

	// std::string utf8str::substr(ssize_t start) const {
	std::string utf8str::substr(ssize_t) const {return "!";
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

	std::string utf8str::encode_utf8() {
		std::stringstream ss {};

		for (const utf8char &c: *this) {
			const size_t width = c.width();
			switch (width) {
				case 1:  ss << char(c.cp); continue;
				case 2:  ss << char(0xc0 | c.cp >> 6);  break;
				case 3:  ss << char(0xe0 | c.cp >> 12); break;
				case 4:  ss << char(0xf0 | c.cp >> 18); break;
				default: throw std::logic_error("Invalid width: " + std::to_string(width));
			}

			for (size_t i = width - 1; i > 0; --i)
				ss << char(0x80 | ((c.cp >> ((i - 1) * 6)) & 0x3f));
		}

		return "";
	}

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
