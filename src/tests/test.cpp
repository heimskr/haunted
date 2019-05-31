#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include <cassert>

#include "lib/ansi.h"
#include "tests/test.h"
#include "core/key.h"
#include "core/util.h"

namespace haunted::tests {
	std::string testing::stringify(const std::string &str) {
		return "\"" + str + "\"";
	}

	std::string testing::stringify(bool b) {
		return b? "T" : "F";
	}

	void maintest::test_csiu() {
		ansi::out << "\nTesting CSI u checking.\n";
		testing::check<std::string, bool>({
			{"1;1u",   true},
			{"1;1a",  false},
			{";1u",   false},
			{"1;u",   false},
			{"4u",    false},
			{"1;u1",  false},
			{"1u;1",  false},
			{";1u",   false},
			{"u1;1",  false},
			{"42;0u",  true},
			{"3;911u", true},
			{"5;5U",  false},
		}, &util::is_csiu, "is_csiu");
	}
}

int main(int, char **) {
	using namespace haunted;
	terminal term;
	term.cbreak();

	haunted::tests::maintest::test_csiu();

	std::cout << "\n";

	key k;
	while (term >> k) {
		std::cout << "\r" << std::setw(3) << std::left << std::setfill(' ') << int(k) << " " << k << std::endl;
		if (k == '\\')
			break;
		else if (k == '.')
			term.raw = !term.raw;
	}

}
