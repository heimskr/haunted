#ifndef HAUNTED_LIB_SUPERSTRING_H_
#define HAUNTED_LIB_SUPERSTRING_H_

#include <list>
#include <string>

namespace haunted {
	/** Represents a character made of any number of codepoints. */
	using superchar = std::string;

	/** Used for creating an easily editable string that can accommodate characters containing multiple codepoints (such
	 *  as some emoji). */
	class superstring {
		private:
			std::list<superchar> chunks;
			using iterator = decltype(chunks)::iterator;

			iterator nth(size_t);

		public:
			/** Assembles a new superstring from a UTF-8 string. */
			superstring(const std::string &);
			superstring(const char *str): superstring(std::string(str)) {}
			superstring(char ch, size_t n = 1): superstring(std::string(n, ch)) {}

			/** Joins the substrings into a single string. */
			std::string str() const;

			/** Converts the superstring into a string. */
			operator std::string() const { return str(); }

			/** Accesses a substring. */
			superchar & operator[](ssize_t);

			/** Accesses a substring. */
			superchar & at(size_t);

			std::string substr(size_t, size_t) const;
			void insert(size_t, const superchar &);
			void insert(size_t, char);
			superstring & erase(size_t = 0, size_t = std::string::npos);

			void dbg();

			iterator begin() { return chunks.begin(); }
			iterator end()   { return chunks.end(); }

			/** Returns the number of substrings. */
			size_t size()   const { return chunks.size(); }
			size_t length() const { return chunks.size(); }

			/** Returns whether the superstring is empty. */
			bool empty() const { return chunks.empty(); }

			/** Returns the sum of all the substrings' lengths. */
			size_t text_length() const;

			void clear() { chunks.clear(); }
	};
}

#endif
