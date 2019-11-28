#ifndef HAUNTED_LIB_USTRING_H_
#define HAUNTED_LIB_USTRING_H_

#include <string>

#include <unicode/ustring.h>
#include <unicode/brkiter.h>

#define USTRING_WIDTH_AT_BUFFER_SIZE 10

namespace haunted {
	/**
	 * This is a wrapper class for icu::UnicodeString that indexes strings by graphemes rather than codepoints.
	 */
	class ustring {
		private:
			icu::UnicodeString data;
			size_t length_;
			size_t & scan_length();

		public:
			ustring(const char *);
			ustring(const std::string &);
			ustring(const icu::UnicodeString &);

			icu::UnicodeString & get_data() { return data; }

			void check_index(size_t) const;

			ustring raw_substr(size_t, size_t = std::string::npos) const;
			ustring substr(size_t, size_t = std::string::npos) const;

			size_t length() const;
			bool empty() const;

			ustring & insert(size_t, const ustring &);
			ustring & insert(size_t, char16_t);
			ustring & erase(size_t, size_t = std::string::npos);

			std::string at(size_t) const;

			/** Returns the width (in columns) of the character at a given index. */
			size_t width_at(size_t) const;

#include "ustring.tcc"

			bool operator==(const std::string &) const;
			bool operator!=(const std::string &) const;
			std::string operator[](size_t) const;
			operator std::string() const;
			friend std::ostream & operator<<(std::ostream &, const ustring &);

			class iterator {
				friend class ustring;

				private:
					const ustring &ustr;
					icu::BreakIterator *bi;
					icu::Locale locale;
					size_t prev, pos;
					iterator(const ustring &, const icu::Locale & = icu::Locale::getUS());
					iterator & end();

				public:
					iterator(const iterator &);
					~iterator();
					iterator & operator++();
					iterator & operator--();
					iterator & operator+=(ssize_t);
					iterator & operator-=(ssize_t);
					iterator operator+(ssize_t) const;
					iterator operator-(ssize_t) const;
					std::string operator*();
					bool operator==(const iterator &) const;
					bool operator!=(const iterator &) const;
			};

			iterator begin() const;
			iterator end() const;
			iterator begin(const icu::Locale &) const;
			iterator end(const icu::Locale &) const;
	};

	ustring::iterator operator+(ssize_t, const ustring::iterator &);
	std::ostream & operator<<(std::ostream &, const ustring &);
}

#endif
