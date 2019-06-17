#include "superstring.h"
#include "utf8.h"

namespace haunted {
	superstring::superstring(const std::string &str) {
		size_t length = str.length();
		for (size_t i = 0; i < length; ++i) {
			unsigned char c = str[i];
			if (c < 0x80) {
				chunks.push_back(std::string(1, c));
			} else {
				chunks.push_back(str.substr(i, utf8::width(c)));
			}
		}

		cursor = chunks.begin();
	}

	superstring::iterator & superstring::move(size_t pos) {
		if (pos == 0) {
			index = 0;
			cursor = chunks.begin();
		} else if (pos < index) {
			for (; index < pos; ++index, ++cursor);
		} else if (index < pos) {
			for (; pos < index; --index, --cursor);
		}

		return cursor;
	}

	void superstring::next() {
		++index, ++cursor;
	}

	superstring::operator std::string() const {
		std::string out;
		for (const std::string &sub: chunks)
			out.append(sub);
		return out;
	}

	superchar & superstring::operator[](ssize_t pos) {
		if (pos < 0) {
			for (ssize_t i = 0; pos < i; --i)
				chunks.push_front({});
			move();
		} else if (size() <= size_t(pos)) {
			for (ssize_t i = size(); i <= pos; ++i) {
				chunks.push_back({});
				next();
			}
		} else {
			move(pos);
		}

		return *cursor;
	}

	superchar & superstring::at(size_t pos) {
		if (size() <= pos)
			throw std::out_of_range("superstring");

		move(pos);
		return *cursor;
	}

	std::string superstring::substr(size_t pos, size_t n) const {
		return std::string(*this).substr(pos, n);
	}

	superchar & superstring::insert(size_t pos, const superchar &item) {
		move(pos);
		return *chunks.insert(++cursor, item);
	}

	superchar & superstring::insert(size_t pos, char ch) {
		return insert(pos, superchar(1, ch));
	}

	superstring & superstring::erase(size_t pos, size_t len) {
		superstring::iterator last = move(pos);

		if (len == std::string::npos || size() < pos + len) {
			last = chunks.end();
		} else {
			for (size_t i = 0; i < len; ++i, ++last);
		}

		chunks.erase(cursor, last);
		return *this;
	}


	superstring::iterator superstring::begin() { return chunks.begin(); }
	superstring::iterator superstring::end()   { return chunks.end();   }
	size_t superstring::size()   const { return chunks.size();  }
	size_t superstring::length() const { return chunks.size();  }
	void superstring::clear() { chunks.clear(); }
}
