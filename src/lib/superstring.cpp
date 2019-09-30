#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>

#include "haunted/core/defs.h"

#include "lib/superstring.h"
#include "lib/utf8.h"
#include "lib/formicine/ansi.h"

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
	}

	superstring::iterator superstring::nth(size_t pos) {
		iterator iter = begin();
		for (size_t i = 0; i < pos; ++i, ++iter);
		return iter;
	}

	std::string superstring::str() const {
		std::string out;
		for (const std::string &sub: chunks)
			out.append(sub);
		return out;
	}

	superchar & superstring::operator[](ssize_t pos) {
		if (pos < 0) {
			for (ssize_t i = 0; pos < i; --i)
				chunks.push_front({});
			return *chunks.begin();
		} else if (size() <= size_t(pos)) {
			for (ssize_t i = size(); i <= pos; ++i)
				chunks.push_back({});
			return *--chunks.end();
		}

		return *nth(pos);
	}

	superchar & superstring::at(size_t pos) {
		if (size() <= pos)
			throw std::out_of_range("superstring");

		return *nth(pos);
	}

	std::string superstring::substr(size_t pos, size_t n) const {
		return std::string(*this).substr(pos, n);
	}

	void superstring::insert(size_t pos, const superchar &item) {
		if (pos == size()) {
			chunks.push_back(item);
		} else if (pos == 0) {
			chunks.push_front(item);
		} else {
			chunks.insert(nth(pos), item);
		}
	}

	void superstring::insert(size_t pos, char ch) {
		insert(pos, superchar(1, ch));
	}

	void superstring::dbg() {
		std::string str = "[" + std::to_string(size()) + "]";
		for (const superchar &sc: chunks)
			str += " \"" + sc + "\"";
	}

	superstring & superstring::erase(size_t pos, size_t len) {
		if (size() == 0 && pos == 0)
			return *this;

		// ansi::jump(0, 4 - x);
		// std::cout << "[" << size() << " @ " << index << "] erase(" << pos << ", " << len << ")";


		// DBG("[" << size() << "] erase(" << pos << ", " << len << ")");

		// DBG("--- begin = " << &*begin() << ", cursor = " << &*cursor << ", last = " << &*last << ", index = " << index << ", end = " << &*end());
		iterator iter = nth(pos);
		iterator last = iter;
		for (size_t i = 0; i < len; ++i, ++last);

		chunks.erase(iter, last);

		return *this;
	}

	size_t superstring::text_length() const {
		size_t out = 0;
		for (const superchar &sc: chunks)
			out += sc.length();
		return out;
	}
}
