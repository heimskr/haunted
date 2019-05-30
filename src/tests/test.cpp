#include <iostream>
#include <iomanip>

#include "tests/test.h"
#include "core/terminal.h"
#include "core/key.h"

int main(int, char **) {
	using namespace haunted;
	terminal term;
	term.cbreak();

	key k;
	while (term >> k) {
		std::cout << "\r" << std::setw(3) << std::left << std::setfill(' ') << static_cast<int>(k) << " " << k << std::endl;
		if (k == '|')
			break;
	}

	std::cout << "Terminating." << std::endl;
}
