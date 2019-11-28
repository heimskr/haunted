#include "lib/ustring.h"
#include "lib/formicine/ansi.h"

namespace haunted {
	ustring::ustring(const char *str) {
		data = icu::UnicodeString::fromUTF8(str);
		scan_length();
	}

	ustring::ustring(const std::string &str) {
		data = icu::UnicodeString::fromUTF8(str);
		scan_length();
	}

	ustring::ustring(const icu::UnicodeString &ustr): data(ustr) {
		scan_length();
	}

	size_t & ustring::scan_length() {
		length_ = 0;
		for (ustring::iterator iter = begin(), end_ = end(); iter != end_; ++iter, ++length_);
		return length_;
	}

	void ustring::check_index(size_t index) const {
		if (length_ < index) {
			throw std::out_of_range("Invalid index: " + std::to_string(index) + " (length is " + std::to_string(length_)
				+ ")");
		}
	}

	ustring ustring::raw_substr(size_t start, size_t length) const {
		return ustring(data.tempSubString(start, length));
	}

	ustring ustring::substr(size_t start, size_t length) const {
		check_index(start);

		if (length == 0)
			return "";

		ustring::iterator iter = begin();

		for (size_t i = 0; i < start; ++i)
			++iter;

		icu::UnicodeString raw;
		ustring::iterator end_ = end();
		for (size_t i = 0; i < length && iter != end_; ++i) {
			raw.append(data.tempSubString(iter.prev, iter.pos - iter.prev));
			++iter;
		}

		return ustring(raw);
	}

	size_t ustring::length() const {
		return length_;
	}

	bool ustring::empty() const {
		return data.isEmpty();
	}
	
	ustring & ustring::insert(size_t pos, const ustring &str) {
		ustring::iterator iter = begin();
		iter += pos;
		data.insert(iter.prev, str.data);
		length_ += str.length_;
		return *this;
	}
	
	ustring & ustring::insert(size_t pos, char16_t ch) {
		ustring::iterator iter = begin();
		iter += pos;
		data.insert(iter.prev, ch);
		++length_;
		return *this;
	}

	std::string ustring::at(size_t index) const {
		// Not pretty.
		return substr(index, 1UL);
	}

// Operators

	bool ustring::operator==(const std::string &str) const {
		return std::string(*this) == str;
	}

	bool ustring::operator!=(const std::string &str) const {
		return std::string(*this) != str;
	}

	std::string ustring::operator[](size_t index) const {
		return at(index);
	}

	ustring::operator std::string() const {
		std::string out;
		data.toUTF8String(out);
		return out;
	}

	std::ostream & operator<<(std::ostream &os, const ustring &us) {
		return os << std::string(us);
	}

// Iterator

	ustring::iterator::iterator(const ustring &ustr_, const icu::Locale &locale_): ustr(ustr_), locale(locale_) {
		UErrorCode code = U_ZERO_ERROR;
		bi = icu::BreakIterator::createCharacterInstance(locale, code);
		if (0 < code) {
			throw std::runtime_error("icu::BreakIterator::createCharacterInstance returned error code " +
				std::to_string(code));
		}

		bi->setText(ustr.data);
		prev = 0;
		pos = bi->next();
	}

	ustring::iterator::iterator(ustring::iterator &iter): ustr(iter.ustr) {
		bi = iter.bi->clone();
		locale = iter.locale;
		prev = iter.prev;
		pos = iter.pos;
	}

	ustring::iterator::~iterator() {
		delete bi;
	}

	ustring::iterator & ustring::iterator::operator++() {
		prev = pos;
		pos = bi->next();
		return *this;
	}

	ustring::iterator & ustring::iterator::operator--() {
		pos = prev;
		prev = bi->previous();
		return *this;
	}

	ustring::iterator & ustring::iterator::operator+=(ssize_t offset) {
		if (offset < 0)
			return *this -= -offset;

		if (offset == 0)
			return *this;

		if (offset == 1)
			return ++*this;

		prev = bi->next(offset - 1);
		pos = bi->next();
		return *this;
	}

	ustring::iterator & ustring::iterator::operator-=(ssize_t offset) {
		if (offset < 0)
			return *this += -offset;

		if (offset == 0)
			return *this;

		if (offset == 1)
			return --*this;

		for (ssize_t i = 0; i < offset; ++i)
			bi->previous();

		prev = bi->previous();
		pos = bi->next();
		return *this;
	}

	std::string ustring::iterator::operator*() {
		return ustr.raw_substr(prev, pos - prev);
	}

	bool ustring::iterator::operator==(const ustring::iterator &rhs) const {
		return &ustr == &rhs.ustr &&
			((pos == rhs.pos && prev == rhs.prev) || (pos == std::string::npos && prev == rhs.prev));
	}

	bool ustring::iterator::operator!=(const ustring::iterator &rhs) const {
		return &ustr != &rhs.ustr ||
			((pos != rhs.pos || prev != rhs.prev) && (pos != std::string::npos || prev != rhs.prev));
	}

	ustring::iterator & ustring::iterator::end() {
		prev = pos = bi->last();
		return *this;
	}

	ustring::iterator ustring::begin() const {
		return ustring::iterator(*this);
	}

	ustring::iterator ustring::end() const {
		return ustring::iterator(*this).end();
	}

	ustring::iterator ustring::begin(const icu::Locale &locale) const {
		return ustring::iterator(*this, locale);
	}

	ustring::iterator ustring::end(const icu::Locale &locale) const {
		return ustring::iterator(*this, locale).end();
	}
}
