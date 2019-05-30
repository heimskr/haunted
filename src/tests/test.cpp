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
		std::cout << "\r" << std::setw(3) << std::left << std::setfill(' ') << int(k) << " " << k << std::endl;
		if (k == '\\')
			break;
		else if (k == '.')
			term.raw = !term.raw;
	}

	std::cout << "Terminating." << std::endl;
}
