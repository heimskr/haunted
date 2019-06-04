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
#include "ui/textinput.h"

namespace haunted::tests {
	/**
	 * Stringifies a pair of integers.
	 */
	std::string testing::stringify(const std::pair<int, int> &p) {
		return "{" + std::to_string(p.first) + ", " + std::to_string(p.second) + "}";
	}

	/**
	 * Stringifies a string by surrounding it with double quotes.
	 */
	std::string testing::stringify(const std::string &str) {
		return "\"" + str + "\"";
	}

	/**
	 * Stringifies a bool into a single letter (T or F).
	 */
	std::string testing::stringify(bool b) {
		return b? "T" : "F";
	}

	/**
	 * Runs some tests for the CSI u functions.
	 */
	void maintest::test_csiu() {
		ansi::out << "\nTesting CSI u validation.\n";
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

		ansi::out << "\nTesting CSI u parsing.\n";
		testing::check<std::string, std::pair<int, int>>({
			{"1;1u",    { 1,   1}},
			{"42;0u",   {42,   0}},
			{"3;911u",  { 3, 911}},
			{"55;555u", {55, 555}},
			{"1;1a",    {-1,  -2}},
			{";1u",     {-1,  -1}},
			{"1;u",     {-1,  -1}},
			{"4u",      {-1,  -1}},
			{"1;u1",    {-1,  -2}},
			{"1u;1",    {-1,  -2}},
			{";1u",     {-1,  -1}},
			{"u1;1",    {-1,  -1}},
			{"5;5U",    {-1,  -2}},
		}, &util::parse_csiu, "parse_csiu");
	}

	void maintest::test_textinput(terminal &term) {
		haunted::ui::textinput ti(&term);
		
	}
}

int main(int, char **) {
	using namespace haunted;
	
	terminal term;
	term.cbreak();
	term.watch_size();
	term.start_input();

	// haunted::tests::maintest::test_csiu();
	haunted::tests::maintest::test_textinput(term);

	std::cout << "\n";

	// key k;
	// while (term >> k) {
	// 	std::cout <<  "\r"  << std::setw(3) << std::left << std::setfill(' ')
	// 	          << int(k) <<  " "  <<  k  << std::endl;
	// 	if (k == '\\')
	// 		break;
	// 	else if (k == '.')
	// 		term.raw = !term.raw;
	// }

}
