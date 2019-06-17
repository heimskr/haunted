#include <fstream>
#include <sstream>
#include <iostream>
#include "core/defs.h"
#include <unistd.h>

#include "superstring.h"
#include "utf8.h"
#include "formicine/ansi.h"

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
		DBG("move(" << pos << "): index = " << index << ", begin = " << &*begin() << "(" << *begin() << "), cursor = " << &*cursor << ", end = " << &*end());
		if (pos == 0) {
			DBG("    pos == 0");
			index = 0;
			cursor = chunks.begin();
		} else if (index < pos) {
			DBG("    index < pos");
			for (; index < pos; ++index) {
				void *old = &*cursor;
				void *neu = &*++cursor;
				DBG("    (cursor: ++" << old << " → " << neu << ")");
			}
		} else if (pos < index) {
			DBG("    pos < index");
			for (; pos < index; --index) {
				void *old = &*cursor;
				void *neu = &*--cursor;
				DBG("    (cursor: --" << old << " → " << neu << ")");
			}
		}

		DBG("--- index = " << index << ", begin = " << &*begin() << "(" << *begin() << "), cursor = " << &*cursor << ", end = " << &*end());
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
		DBG("[" << size() << ":" << length() << "] substr(" << pos << ", " << n << ")");
		return std::string(*this).substr(pos, n);
	}

	void superstring::insert(size_t pos, const superchar &item) {
		DBG("insert(" << pos << ", \"" << item << "\"): [" << size() << "] index = " << index);
		if (pos == size()) {
			DBG("    pos == size(" << size() << ")");
			index = size();
			chunks.push_back(item);
			cursor = chunks.end();
			--cursor;
		} else if (pos == 0) {
			DBG("    pos == 0");
			index = 0;
			chunks.push_front(item);
			cursor = chunks.begin();
		} else {
			DBG("    pos != size(" << size() << "), pos != 0");
			move(pos);
			chunks.insert(++cursor, item);
		}
		dbg();
	}

	void superstring::insert(size_t pos, char ch) {
		insert(pos, superchar(1, ch));
	}

	void superstring::dbg() {
		std::string str = "[" + std::to_string(size()) + "]";
		for (const superchar &sc: chunks) {
			str += " \"" + sc + "\"";
		}

		DBG(str);

		// for (auto iter = begin(); iter != end(); ++iter) {

		// }
	}

	superstring & superstring::erase(size_t pos, size_t len) {
		// static std::ofstream log("erase.log");
		dbg();

		if (size() == 0 && pos == 0)
			return *this;

		
		superstring::iterator prev = cursor;
		DBG("((erase: [begin, cursor, end] = [" << &*begin() << ", " << &*cursor << ", " << &*end() << "]))");
		--prev;
		size_t old_index = index;

		// ansi::jump(0, 4 - x);
		// std::cout << "[" << size() << " @ " << index << "] erase(" << pos << ", " << len << ")";






		DBG("[" << size() << " @ " << index << "] erase(" << pos << ", " << len << "): begin = " << &*begin() << ", cursor = " << &*cursor << ", index = " << index << ", end = " << &*end());

		superstring::iterator last = move(pos);

		DBG("    cursor: " << &*cursor << ", index = " << index << ", last: " << &*last);

		if (len == std::string::npos || size() < pos + len) {
			last = chunks.end();
		} else {
			// for (size_t i = 0; i < len; ++i, ++last);
			for (size_t i = 0; i < len; ) {
				DBG("    (i: " << i << " → " << i + 1 << ")");
				++i;
				DBG("    last: " << &*last);
				++last;
				DBG("        → " << &*last);
			}
		}

		if (pos == old_index) {
			cursor = prev;
			index = old_index - 1;
		} else if (pos < old_index) {
			move(0);
		}


		DBG("--- begin = " << &*begin() << ", cursor = " << &*cursor << ", last = " << &*last << ", index = " << index << ", end = " << &*end());

		chunks.erase(cursor, last);

		return *this;
	}


	superstring::iterator superstring::begin() { return chunks.begin(); }
	superstring::iterator superstring::end()   { return chunks.end();   }
	size_t superstring::size()   const { return chunks.size();  }
	
	size_t superstring::length() const {
		size_t out = 0;
		for (const superchar &sc: chunks)
			out += sc.length();
		return out;
	}

	void superstring::clear() { chunks.clear(); }
}
