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

			// std::list doesn't support random access, so we have to move through the list manually. After one access,
			// the next access will probably happen somewhere close by. If we remember the index of the last access and
			// an iterator pointing to it, we can start there instead of the beginning of the list when searching while
			// doing the next access.
			size_t index = 0;
			iterator cursor;
			iterator & move(size_t = 0);
			void next();

		public:
			/** Assembles a new superstring from a UTF-8 string. */
			superstring(const std::string &);
			superstring(const char *str): superstring(std::string(str)) {}
			superstring(char ch, size_t n = 1): superstring(std::string(n, ch)) {}

			/** Joins the substrings into a single string. */
			operator std::string() const;
			superchar & operator[](ssize_t);

			superchar & at(size_t);

			std::string substr(size_t, size_t) const;
			superchar & insert(size_t, const superchar &);
			superchar & insert(size_t, char);
			superstring & erase(size_t = 0, size_t = std::string::npos);

			iterator begin();
			iterator end();
			size_t size() const;
			size_t length() const;
			void clear();
	};
}

#endif
