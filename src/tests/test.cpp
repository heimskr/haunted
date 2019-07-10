#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include <cassert>
#include <unistd.h>
#include <fcntl.h>

#include "../lib/formicine/ansi.h"
#include "tests/test.h"
#include "core/key.h"
#include "core/util.h"
#include "core/terminal.h"
#include "ui/textinput.h"

namespace haunted::tests {
	/** Stringifies a pair of integers. */
	std::string testing::stringify(const std::pair<int, int> &p) {
		return "{" + std::to_string(p.first) + ", " + std::to_string(p.second) + "}";
	}

	/** Stringifies a string by surrounding it with double quotes. */
	std::string testing::stringify(const std::string &str) {
		return "\"" + str + "\"";
	}

	/** Stringifies a bool into a single letter (T or F). */
	std::string testing::stringify(bool b) {
		return b? "T" : "F";
	}

	/** Runs some tests for the CSI u functions. */
	void maintest::test_csiu() {
		ansi::out << "\nTesting CSI u validation.\n";
		// testing::check<std::string, bool>({
		// 	{"1;1u",   true},
		// 	{"1;1a",  false},
		// 	{";1u",   false},
		// 	{"1;u",   false},
		// 	{"4u",    false},
		// 	{"1;u1",  false},
		// 	{"1u;1",  false},
		// 	{";1u",   false},
		// 	{"u1;1",  false},
		// 	{"42;0u",  true},
		// 	{"3;911u", true},
		// 	{"5;5U",  false},
		// }, &util::is_csiu, "is_csiu");

		// ansi::out << "\nTesting CSI u parsing.\n";
		// testing::check<std::string, std::pair<int, int>>({
		// 	{"1;1u",    { 1,   1}},
		// 	{"42;0u",   {42,   0}},
		// 	{"3;911u",  { 3, 911}},
		// 	{"55;555u", {55, 555}},
		// 	{"1;1a",    {-1,  -2}},
		// 	{";1u",     {-1,  -1}},
		// 	{"1;u",     {-1,  -1}},
		// 	{"4u",      {-1,  -1}},
		// 	{"1;u1",    {-1,  -2}},
		// 	{"1u;1",    {-1,  -2}},
		// 	{";1u",     {-1,  -1}},
		// 	{"u1;1",    {-1,  -1}},
		// 	{"5;5U",    {-1,  -2}},
		// }, &util::parse_csiu, "parse_csiu");
	}

	void maintest::test_textinput(terminal &term) {
		haunted::ui::textinput *ti = new haunted::ui::textinput(&term);
		term.set_root(ti);
		ti->focus();
		ti->resize({0, 0, term.get_cols(), 1});
		term.cbreak();
		term.start_input();
	}

	void maintest::test_key(terminal &term) {
		key k;
		term.cbreak();
		while (term >> k) {
			std::cout <<  "\r"  << std::setw(3) << std::left << std::setfill(' ')
			          << int(k) <<  " "  <<  k  << std::endl;
			if (k == '\\')
				break;
			else if (k == '.')
				term.raw = !term.raw;
		}
	}

	void maintest::test_cursor(terminal &term) {
		ansi::clear();
		ansi::jump(180, 0);
		term << "0";
		ansi::jump(181, 1);
		term << "1";
		term.flush();
	}

	void maintest::test_margins(terminal &term) {
		term.cbreak();
		ansi::clear();
		ansi::jump();
		const std::string spam = "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		for (int i = 0; i < 40; ++i) {
			std::cout << spam.substr(i % spam.length(), std::string::npos) << spam.substr(0, i % spam.length()) << "\n";
		}

		sleep(1); std::cerr << "Setting margins.\n";
		term.enable_hmargins();
		term.margins(2, 10, 5, 15);
		
		sleep(1); std::cerr << "Scrolling up.\n";
		ansi::scroll_up(2);
		
		sleep(1); std::cerr << "Scrolling down.\n";
		ansi::scroll_down(1);

		sleep(1); std::cerr << "Jumping and printing.\n";
		ansi::jump(5, 2);
		term << "Hello";
		term.flush();

		sleep(1); std::cerr << "Clearing line.\n";
		ansi::clear_line();

		sleep(1); std::cerr << "Clearing all.\n";
		ansi::clear();

		sleep(1); std::cerr << "Resetting margins.\n";
		term.margins();

		sleep(5); std::cerr << "Scrolling up.\n";
		ansi::scroll_up(2);

		sleep(5); std::cerr << "Done.\n";
		term.disable_hmargins();
	}
}

int main(int argc, char **argv) {
	using namespace haunted;

	terminal term;
	term.watch_size();

	int fd = open(".log", O_RDWR | O_APPEND);
	dup2(fd, 2);

	if (argc < 2) {
		haunted::tests::maintest::test_key(term);
		return EXIT_SUCCESS;
	}
	
	std::string arg(argv[1]);
	if (arg == "key") {
		haunted::tests::maintest::test_key(term);
	} else if (arg == "csiu") {
		haunted::tests::maintest::test_csiu();
	} else if (arg == "input") {
		haunted::tests::maintest::test_textinput(term);
	} else if (arg == "cursor") {
		haunted::tests::maintest::test_cursor(term);
	} else if (arg == "margins") {
		haunted::tests::maintest::test_margins(term);
	} else {
		haunted::tests::maintest::test_key(term);
	}
}
