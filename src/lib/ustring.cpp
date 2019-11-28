#include "lib/ustring.h"
#include "lib/formicine/ansi.h"

namespace haunted {
	ustring::ustring(const std::string &str) {
		data = icu::UnicodeString::fromUTF8(str);
	}

	ustring::ustring(const icu::UnicodeString &ustr): data(ustr) {}

	ustring ustring::substr(size_t start, size_t length) {
		return ustring(data.tempSubString(start, length));
	}

	size_t ustring::length() const {
		return data.length();
	}

	bool ustring::empty() const {
		return data.isEmpty();
	}
	
	ustring & ustring::insert(size_t pos, const ustring &str) {
		data.insert(pos, str.data);
		return *this;
	}
	
	ustring & ustring::insert(size_t pos, char16_t ch) {
		data.insert(pos, ch);
		return *this;
	}

	ustring::operator std::string() const {
		std::string out;
		data.toUTF8String(out);
		return out;
	}

	ustring::iterator::iterator(ustring &ustr_, const icu::Locale &locale_): ustr(ustr_), locale(locale_) {
		UErrorCode code = U_ZERO_ERROR;
		bi = icu::BreakIterator::createCharacterInstance(locale, code);
		DBG("ustr[" << ustr_ << "]");
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

	std::string ustring::iterator::operator*() {
		return ustr.substr(prev, pos - prev);
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

	ustring::iterator ustring::begin() {
		return ustring::iterator(*this);
	}

	ustring::iterator ustring::end() {
		return ustring::iterator(*this).end();
	}

	ustring::iterator ustring::begin(const icu::Locale &locale) {
		return ustring::iterator(*this, locale);
	}

	ustring::iterator ustring::end(const icu::Locale &locale) {
		return ustring::iterator(*this, locale).end();
	}

	std::ostream & operator<<(std::ostream &os, const ustring &us) {
		return os << std::string(us);
	}
}
