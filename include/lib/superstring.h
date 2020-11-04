#ifndef HAUNTED_LIB_SUPERSTRING_H_
#define HAUNTED_LIB_SUPERSTRING_H_

#include <list>
#include <string>

namespace Haunted {
	/** Represents a character made of any number of codepoints. */
	using Superchar = std::string;

	/** Used for creating an easily editable string that can accommodate characters containing multiple codepoints (such
	 *  as some emoji). */
	class Superstring {
		private:
			std::list<Superchar> chunks;
			using iterator = decltype(chunks)::iterator;

			iterator nth(size_t);

		public:
			/** Assembles a new superstring from a UTF-8 string. */
			Superstring(const std::string &);
			Superstring(const char *str): Superstring(std::string(str)) {}
			Superstring(char ch, size_t n = 1): Superstring(std::string(n, ch)) {}

			/** Joins the substrings into a single string. */
			std::string str() const;

			/** Converts the superstring into a string. */
			operator std::string() const { return str(); }

			/** Accesses a substring. */
			Superchar & operator[](ssize_t);

			/** Accesses a substring. */
			Superchar & at(size_t);

			std::string substr(size_t, size_t) const;
			void insert(size_t, const Superchar &);
			void insert(size_t, char);
			Superstring & erase(size_t = 0, size_t = std::string::npos);

			void dbg();

			iterator begin() { return chunks.begin(); }
			iterator end()   { return chunks.end(); }

			/** Returns the number of substrings. */
			size_t size()   const { return chunks.size(); }
			size_t length() const { return chunks.size(); }

			/** Returns whether the superstring is empty. */
			bool empty() const { return chunks.empty(); }

			/** Returns the sum of all the substrings' lengths. */
			size_t textLength() const;

			void clear() { chunks.clear(); }
	};
}

#endif
