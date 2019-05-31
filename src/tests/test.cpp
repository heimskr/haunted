#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <cassert>

#include "tests/test.h"
#include "core/key.h"
#include "core/util.h"

namespace haunted::tests {
	void maintest::test_csiu() {
		std::vector<std::string> inputs {"1;1u", "1;1a"};
		for (const std::string &input : inputs) {
			std::cout << input << ": " << util::is_csiu(input) << std::endl;
		}

		assert(util::is_csiu("1;1u") == true);
		assert(util::is_csiu("1;1a") == false);

		std::cout << "CSI u tests passed." << std::endl;
	}
}

int main(int, char **) {
	using namespace haunted;
	terminal term;
	term.cbreak();

	haunted::tests::maintest::test_csiu();

	// key k;
	// while (term >> k) {
	// 	std::cout << "\r" << std::setw(3) << std::left << std::setfill(' ') << int(k) << " " << k << std::endl;
	// 	if (k == '\\')
	// 		break;
	// 	else if (k == '.')
	// 		term.raw = !term.raw;
	// }

	std::cout << "Terminating." << std::endl;
}
