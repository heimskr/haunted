#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include <cassert>
#include <unistd.h>
#include <fcntl.h>

#include "../../lib/formicine/ansi.h"
#include "tests/test.h"
#include "core/csi.h"
#include "core/dummy_terminal.h"
#include "core/key.h"
#include "core/util.h"
#include "core/terminal.h"
#include "ui/boxes/simplebox.h"
#include "ui/boxes/expandobox.h"
#include "ui/textbox.h"
#include "ui/textinput.h"

namespace haunted::tests {
	/** Stringifies a pair of integers. */
	std::string testing::stringify(const std::pair<int, int> &p) {
		return "{" + std::to_string(p.first) + ", " + std::to_string(p.second) + "}";
	}

	/** Stringifies a string by surrounding it with double quotes. */
	std::string testing::stringify(const std::string &str) {
		std::string escaped("");
		for (char c: str) {
			switch (c) {
				case '"':  escaped += "\\\""; break;
				case '\n': escaped += "\\n";  break;
				case '\r': escaped += "\\r";  break;
				case '\t': escaped += "\\t";  break;
				default:   escaped += c;
			}
		}

		return "\"" + escaped + "\"";
	}

	/** Stringifies a bool into a single letter (T or F). */
	std::string testing::stringify(bool b) {
		return b? "T" : "F";
	}

	std::pair<int, int> maintest::parse_csi(const std::string &input) {
		haunted::csi testcsi(input);
		return {testcsi.first, testcsi.second};
	}

	/** Runs some tests for the CSI u functions. */
	void maintest::test_csiu() {
		ansi::out << "\nTesting CSI u validation.\n";
		testing unit;

		unit.check<std::string, bool>({
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
		}, &csi::is_csiu, "is_csiu");

		ansi::out << "\nTesting CSI u parsing.\n";
		unit.check<std::string, std::pair<int, int>>({
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
		}, &parse_csi, "parse_csiu");
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

		sleep(2); std::cerr << "Jumping and printing.\n";
		ansi::jump(5, 2);
		term << "Hello";
		term.flush();

		sleep(2); std::cerr << "Setting origin, jumping and printing.\n";
		term.set_origin();
		ansi::jump();
		term << "Hi :)";
		term.flush();

		sleep(2); std::cerr << "Clearing line.\n";
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

	void maintest::test_textbox(terminal &term) {
		using haunted::ui::textline;

		term.cbreak();
		haunted::ui::textbox *tb = new haunted::ui::textbox(&term);
		*tb += "hi";
		*tb += "what's up :)";
		*tb += "third line.";
		term.draw();
		key k;
		while (term >> k) {
			if (k == '\\')
				break;
			if (k == ktype::up_arrow) {
				tb->vscroll(-1);
				tb->draw();
			} else if (k == ktype::down_arrow) {
				tb->vscroll(1);
				tb->draw();
			} else if (k == ktype::left_arrow) {
				tb->set_voffset(-1);
				tb->draw();
			} else if (k == ktype::hash) {
				*tb += textline("This is a very long line. Its purpose is to test the continuation of lines in a textbox. Its continuation value is set to 26, so the wrapped text should line up with the start of the second sentence in the line.", 26);
			} else if (k == ktype::star) {
				for (const textline &line : *tb) {
					DBG(line.continuation << "[" << line.text << "]");
				}
			} else {
				*tb += "Key: [" + std::string(k) + "]";
			}
		}
	}

	void maintest::unittest_expandobox(terminal &) {
		using namespace haunted::ui::boxes;

		testing unit;
		dummy_terminal dummy;

		simplebox wrapper(&dummy);
		wrapper.resize({10, 10, 500, 100});

		ui::textbox tb1(&wrapper);
		ui::textbox tb2(&wrapper);

		expandobox *expando = new expandobox(&wrapper, wrapper.get_position(), horizontal, {{&tb1, 10}, {&tb2, -1}});

		unit.check(wrapper.get_position(),  {10, 10, 500, 100}, "wrapper posiution");
		unit.check(expando->get_position(), {10, 10, 500, 100}, "expando posiution");
		unit.check(tb1.get_position(), {-1, -1, -1, -1}, "tb1 position");
		unit.check(tb2.get_position(), {-1, -1, -1, -1}, "tb2 position");
		expando->resize();
		unit.check(tb1.get_position(), {10, 10, 10,  100}, "tb1 position");
		unit.check(tb2.get_position(), {20, 10, 490, 100}, "tb2 position");
	}

	void maintest::unittest_textbox(terminal &term) {
		using namespace haunted::ui;
		
		testing unit;
		dummy_terminal dummy;

		boxes::simplebox wrapper(&dummy);
		wrapper.resize({0, 0, 20, 10});
		
		textbox *tb = new textbox(&wrapper, wrapper.get_position());

		textline t1("Hello", 4);
		textline t2("This line is longer than the control's width of 20 characters. Its continuation should align with the third word.", 10);
		textline t3("Another line.", 1);
		textline t4("This is another long line with a continuation of 0.");
		textline t5("Exactly 20 chars :^)", 0);

		*tb += t1;
		*tb += t2;
		*tb += t3;
		*tb += t3;
		*tb += t4;
		*tb += t5;

		int rows = tb->total_rows();

		unit.check(tb->line_rows(t2), 11, "line_rows(" + ansi::wrap("t2", ansi::bold) + ")");
		unit.check(rows, 18, "total_rows()");

		unit.check({
			{0,  {t1, 0}},
			{1,  {t2, 0}},
			{2,  {t2, 1}},
			{3,  {t2, 2}},
			{4,  {t2, 3}},
			{5,  {t2, 4}},
			{6,  {t2, 5}},
			{7,  {t2, 6}},
			{8,  {t2, 7}},
			{9,  {t2, 8}},
			{10, {t2, 9}},
			{11, {t2, 10}},
			{12, {t3, 0}},
			{13, {t3, 0}},
			{14, {t4, 0}},
			{15, {t4, 1}},
			{16, {t4, 2}},
			{17, {t5, 0}},
		}, &textbox::line_at_row, tb, "line_at_row");

		unit.check("line_at_row(" + std::to_string(rows) + ")", typeid(std::out_of_range), &textbox::line_at_row, tb,
			"Invalid row index: " + std::to_string(rows), rows);

		unit.check({
			{0,  "Hello               "},
			{1,  "This line is longer "},
			{2,  "          than the c"},
			{3,  "          ontrol's w"},
			{4,  "          idth of 20"},
			{5,  "           character"},
			{6,  "          s. Its con"},
			{7,  "          tinuation "},
			{8,  "          should ali"},
			{9,  "          gn with th"},
			{10, ""},
		}, &textbox::text_at_row, tb, "text_at_row");

		term << ansi::info << "Trying to scroll 10 lines down (current voffset is "
		     << ansi::wrap(std::to_string(tb->voffset), ansi::bold) << ")." << ansi::endl;
		tb->vscroll(10);
		unit.check(tb->voffset, 8, "voffset");

		unit.check({
			{0,  "          should ali"},
			{1,  "          gn with th"},
			{2,  "          e third wo"},
			{3,  "          rd.       "},
			{4,  "Another line.       "},
			{5,  "Another line.       "},
			{6,  "This is another long"},
			{7,  " line with a continu"},
			{8,  "ation of 0.         "},
			{9,  "Exactly 20 chars :^)"},
			{10, ""},
		}, &textbox::text_at_row, tb, "textbox::text_at_row");

		unit.check(tb->next_row(), -1, "next_row()");
		term << ansi::info << "Resetting textbox." << ansi::endl;
		tb->clear_lines();
		unit.check("line_at_row(0)", typeid(std::out_of_range), &textbox::line_at_row, tb, "Invalid row index: 0", 0);
		unit.check(tb->voffset, 0, "voffset");
		unit.check(tb->total_rows(), 0, "total_rows()");
		unit.check(tb->effective_voffset(), 0, "effective_voffset()");
		unit.check(tb->pos.height, 10, "pos.height");
		unit.check(tb->next_row(), 0, "next_row()");
		*tb += t1;
		unit.check(tb->next_row(), 1, "next_row()");
		*tb += t1;
		unit.check(tb->next_row(), 2, "next_row()");
		*tb += t4;
		unit.check(tb->next_row(), 5, "next_row()");
		*tb += t1;
		unit.check(tb->next_row(), 6, "next_row()");
		*tb += t1;
		unit.check(tb->next_row(), 7, "next_row()");
		*tb += t1;
		unit.check(tb->next_row(), 8, "next_row()");
		*tb += t1;
		unit.check(tb->next_row(), 9, "next_row()");
		*tb += t1;
		unit.check(tb->next_row(), -1, "next_row()");

		using namespace std::string_literals;
		unit.check(t1.text_at_row(tb->pos.width, 0), "Hello               "s, "t1.text_at_row(0)");
		unit.check(t2.text_at_row(tb->pos.width, 0), "This line is longer "s, "t2.text_at_row(0)");
		unit.check(t2.text_at_row(tb->pos.width, 1), "          than the c"s, "t2.text_at_row(1)");
		unit.check(t2.text_at_row(tb->pos.width, 2), "          ontrol's w"s, "t2.text_at_row(2)");
		unit.check(t2.text_at_row(tb->pos.width, 3), "          idth of 20"s, "t2.text_at_row(3)");
		unit.check(t2.text_at_row(tb->pos.width, 4), "           character"s, "t2.text_at_row(4)");
		unit.check(t2.text_at_row(tb->pos.width, 5), "          s. Its con"s, "t2.text_at_row(5)");
		unit.check(t2.text_at_row(tb->pos.width, 6), "          tinuation "s, "t2.text_at_row(6)");
		unit.check(t2.text_at_row(tb->pos.width, 7), "          should ali"s, "t2.text_at_row(7)");
		unit.check(t2.text_at_row(tb->pos.width, 8), "          gn with th"s, "t2.text_at_row(8)");
	}

	void testing::display_results() const {
		using namespace ansi;

		if (total_failed == 0 && total_passed == 0) {
			out << warn << "No tests were run.\n";
		} else if (total_failed == 0) {
			if (total_passed == 1)
				out << good << "Test passed.\n";
			else
				out << good << "All " << total_passed << " tests passed.\n";
		} else if (total_passed == 0) {
			if (total_failed == 1)
				out << bad << "Test failed.\n";
			else
				out << bad << "All " << total_failed << " tests failed.\n";
		} else {
			out << warn
			    << "Passed " << wrap(std::to_string(total_passed), green)
			    << ", failed " << wrap(std::to_string(total_failed), red)
			    << " (" << bold << std::setprecision(4)
			    << (total_passed * 100.0 / (total_passed + total_failed)) << "%" >> bold
			    << ")" << std::defaultfloat << endl;
		}
	}

	void testing::display_failed(const std::string &input,  const std::string &actual, const std::string &expected,
	                             const std::string &prefix, const std::string &padding, const std::exception *err) {
		using namespace ansi;
		out << bad << prefix << parens << wrap(input, bold) << padding << wrap(" == ", dim);

		if (err != nullptr)
			out << wrap(wrap(util::demangle_object(err), bold) + ": " + std::string(err->what()), red);
		else
			out << wrap(actual, red);
		
		out << wrap(" Expected: ", dim) << wrap(expected, yellow) << endl;
	}

	void testing::display_passed(const std::string &input, const std::string &actual, const std::string &prefix,
	                             const std::string &padding) {
		using namespace ansi;
		out << good << prefix << parens << wrap(input, bold) << padding << wrap(" == ", dim)
		    << wrap(actual, green) << endl;
	}

	testing::~testing() {
		if (autodisplay) {
			ansi::out << ansi::endl;
			display_results();
		}
	}
}

int main(int argc, char **argv) {
	using namespace haunted;

	terminal term;
	term.watch_size();

	int fd = open(".log", O_RDWR | O_APPEND | O_CREAT);
	dup2(fd, 2);

	if (argc < 2) {
		haunted::tests::maintest::test_key(term);
		return 0;
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
	} else if (arg == "textbox") {
		haunted::tests::maintest::test_textbox(term);
	} else if (arg == "unittextbox") {
		haunted::tests::maintest::unittest_textbox(term);
	} else if (arg == "unitexpandobox") {
		haunted::tests::maintest::unittest_expandobox(term);
	} else {
		haunted::tests::maintest::test_key(term);
	}
}
