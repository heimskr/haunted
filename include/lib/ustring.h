#ifndef HAUNTED_LIB_USTRING_H_
#define HAUNTED_LIB_USTRING_H_

#include <string>

#include <unicode/ustring.h>
#include <unicode/brkiter.h>

namespace haunted {
	/**
	 * This is a wrapper class for icu::UnicodeString.
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

			void check_index(size_t) const;

			ustring raw_substr(size_t, size_t = std::string::npos);
			ustring substr(size_t, size_t = std::string::npos);

			size_t length() const;
			bool empty() const;

			ustring & insert(size_t, const ustring &);
			ustring & insert(size_t, char16_t);

			bool operator==(const std::string &) const;

			operator std::string() const;
			friend std::ostream & operator<<(std::ostream &, const ustring &);

			class iterator {
				friend class ustring;

				private:
					ustring &ustr;
					icu::BreakIterator *bi;
					icu::Locale locale;
					size_t prev, pos;
					iterator(ustring &, const icu::Locale & = icu::Locale::getUS());
					iterator & end();

				public:
					iterator(iterator &);
					~iterator();
					iterator & operator++();
					iterator & operator--();
					iterator & operator+=(ssize_t);
					iterator & operator-=(ssize_t);
					std::string operator*();
					bool operator==(const iterator &) const;
					bool operator!=(const iterator &) const;
			};

			iterator begin();
			iterator end();
			iterator begin(const icu::Locale &);
			iterator end(const icu::Locale &);
	};

	std::ostream & operator<<(std::ostream &, const ustring &);
}

#endif
