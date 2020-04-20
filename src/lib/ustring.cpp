#ifndef DISABLE_ICU

#include <cstring>

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

	ustring::~ustring() {
		delete_cached();
	}

	size_t & ustring::scan_length() {
		length_ = 0;
		for (ustring::iterator iter = begin(), end_ = end(); iter != end_; ++iter, ++length_);
		return length_;
	}

	void ustring::delete_cached() {
		if (cached_cstr)
			delete cached_cstr;
	}

	void ustring::check_index(size_t index) const {
		if (length_ < index) {
			throw std::out_of_range("Invalid index: " + std::to_string(index) + " (length is " + std::to_string(length_)
				+ ")");
		}
	}

	ustring ustring::raw_substr(size_t start, size_t len) const {
		return ustring(data.tempSubString(start, len));
	}

	ustring ustring::substr(size_t start, size_t len) const {
		check_index(start);

		if (len == 0)
			return "";

		ustring::iterator iter = begin();

		for (size_t i = 0; i < start; ++i)
			++iter;

		icu::UnicodeString raw;
		ustring::iterator end_ = end();
		for (size_t i = 0; i < len && iter != end_; ++i) {
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

	const char * ustring::c_str() {
		if (cached_cstr)
			return cached_cstr;
		std::string str = *this;
		const size_t len = str.length() + 1;
		cached_cstr = static_cast<const char *>(std::calloc(len, sizeof(char)));
		std::memcpy(reinterpret_cast<void *>(const_cast<char *>(cached_cstr)), str.c_str(), len);
		return cached_cstr;
	}

	void ustring::clear() {
		delete_cached();
		length_ = 0;
		data.remove();
	}
	
	ustring & ustring::insert(size_t pos, const ustring &str) {
		if (!str.empty()) {
			ustring::iterator iter = begin() + pos;
			data.insert(iter.prev, str.data);
			DBG("inserting [" << str << "] (raw length: " << std::string(str).length() << ")");
			length_ += str.length_;
			delete_cached();
		}

		return *this;
	}
	
	ustring & ustring::insert(size_t pos, char16_t ch) {
		ustring::iterator iter = begin() + pos;
		data.insert(iter.prev, ch);
		++length_;
		delete_cached();
		return *this;
	}

	ustring & ustring::erase(size_t pos, size_t len) {
		if (0 < len) {
			size_t raw_pos, raw_erase = 0;
			ustring::iterator iter = begin() + pos, end_ = end();
			raw_pos = iter.prev;
			size_t to_erase;
			for (to_erase = 0; to_erase < len && iter != end_; ++to_erase, ++iter);
			raw_erase = iter.prev - raw_pos;
			data.remove(raw_pos, raw_erase);
			length_ -= to_erase;
			delete_cached();
		}

		return *this;
	}

	void ustring::pop_back() {
		if (length_ == 0)
			return;
		data.remove(length_ - 1);
	}

	std::string ustring::at(size_t index) const {
		// Not pretty.
		return substr(index, 1UL);
	}

	size_t ustring::width_at(size_t index) const {
		check_index(index);

		ustring::iterator iter = begin() + index;


		std::string piece = at(index);

		UChar32 buf[USTRING_WIDTH_AT_BUFFER_SIZE];
		UErrorCode code = U_ZERO_ERROR;
		raw_substr(iter.prev, iter.pos - iter.prev).data.toUTF32(buf, USTRING_WIDTH_AT_BUFFER_SIZE, code);
		UChar32 ch = buf[0];

		if (0 < code)
			throw std::runtime_error("icu::UnicodeString::toUTF32 returned error code " + std::to_string(code));

		// https://www.rdocumentation.org/packages/stringi/versions/1.4.3/topics/stri_width
		for (UCharCategory type: {U_CONTROL_CHAR, U_ENCLOSING_MARK, U_NON_SPACING_MARK, U_FORMAT_CHAR}) {
			if (u_charType(ch) == type)
				return 0;
		}

		// For some reason, '#', '*' and 0-9 have the UCHAR_EMOJI property.
		if (' ' <= ch && ch <= '~')
			return 1;

		for (auto prop: {UCHAR_EMOJI}) {
			if (u_hasBinaryProperty(ch, prop)) {
				DBG("2: has binary property (" << ch << ", " << prop << ")");
				return 2;
			}
		}

		UEastAsianWidth ea = static_cast<UEastAsianWidth>(u_getIntPropertyValue(ch, UCHAR_EAST_ASIAN_WIDTH));
		if (ea == U_EA_FULLWIDTH || ea == U_EA_WIDE) {
			DBG("2: ea == U_EA_FULLWIDTH || ea == U_EA_WIDE");
			return 2;
		}

		UHangulSyllableType hst = static_cast<UHangulSyllableType>(u_getIntPropertyValue(ch, UCHAR_HANGUL_SYLLABLE_TYPE));
		if (hst == U_HST_VOWEL_JAMO || hst == U_HST_TRAILING_JAMO) {
			DBG("2: hst == U_HST_VOWEL_JAMO || hst == U_HST_TRAILING_JAMO");
			return 2;
		}

		if (ch == 0x200B) // ZERO WIDTH SPACE (U+200B)
			return 0;

		return 1;
	}

	size_t ustring::width_until(size_t index, size_t offset) const {
		size_t total_width = 0;
		for (size_t i = offset; i < index && i < length_; ++i) {
			// DBG("width('" << at(i) << "') == " << width_at(i));
			total_width += width_at(i);
		}
		return total_width;
	}

	size_t ustring::width() const {
		return width_until(length_, 0);
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

	ustring::iterator::iterator(const ustring::iterator &iter): ustr(iter.ustr) {
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

	ustring::iterator ustring::iterator::operator+(ssize_t to_add) const {
		ustring::iterator copy = *this;
		copy += to_add;
		return copy;
	}

	ustring::iterator operator+(ssize_t to_add, const ustring::iterator &iter) {
		return iter + to_add;
	}

	ustring::iterator ustring::iterator::operator-(ssize_t to_subtract) const {
		ustring::iterator copy = *this;
		copy -= to_subtract;
		return copy;
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

#endif