// #define NODEBUG

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include <cassert>
#include <unistd.h>
#include <fcntl.h>

#include "lib/formicine/ansi.h"
#include "haunted/tests/test.h"
#include "haunted/core/csi.h"
#include "haunted/core/dummy_terminal.h"
#include "haunted/core/key.h"
#include "haunted/core/util.h"
#include "haunted/core/terminal.h"
#include "haunted/ui/boxes/simplebox.h"
#include "haunted/ui/boxes/expandobox.h"
#include "haunted/ui/label.h"
#include "haunted/ui/textbox.h"
#include "haunted/ui/textinput.h"

#ifdef NODEBUG
#define INFO(x)
#else
#define INFO(x) ansi::out << ansi::info << x << ansi::endl
#endif

namespace haunted::tests {
	std::pair<int, int> maintest::parse_csi(const std::string &input) {
		haunted::csi testcsi(input);
		return {testcsi.first, testcsi.second};
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
		term.out_stream.clear();
		term.jump(180, 0);
		term << "0";
		term.jump(181, 1);
		term << "1";
		term.flush();
	}

	void maintest::test_margins(terminal &term) {
		term.cbreak();
		term.out_stream.clear();
		term.out_stream.jump();
		const std::string spam = "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		for (int i = 0; i < 40; ++i) {
			std::cout << spam.substr(i % spam.length(), std::string::npos) << spam.substr(0, i % spam.length()) << "\n";
		}

		sleep(1); std::cerr << "Setting margins.\n";
		term.enable_hmargins();
		term.margins(2, 10, 5, 15);
		
		sleep(1); std::cerr << "Scrolling up.\n";
		term.out_stream.scroll_up(2);
		
		sleep(1); std::cerr << "Scrolling down.\n";
		term.out_stream.scroll_down(1);

		sleep(2); std::cerr << "Jumping and printing.\n";
		term.out_stream.jump(5, 2);
		term << "Hello";
		term.flush();

		sleep(2); std::cerr << "Setting origin, jumping and printing.\n";
		term.set_origin();
		term.out_stream.jump();
		term << "Hi :)";
		term.flush();

		sleep(2); std::cerr << "Clearing line.\n";
		term.out_stream.clear_line();

		sleep(1); std::cerr << "Clearing all.\n";
		term.out_stream.clear();

		sleep(1); std::cerr << "Resetting margins.\n";
		term.margins();

		sleep(5); std::cerr << "Scrolling up.\n";
		term.out_stream.scroll_up(2);

		sleep(5); std::cerr << "Done.\n";
		term.disable_hmargins();
	}

	void maintest::test_textbox(terminal &term) {
		using haunted::ui::textline, haunted::ui::simpleline;

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
				*tb += simpleline("This is a very long line. Its purpose is to test the continuation of lines in a textbox. Its continuation value is set to 26, so the wrapped text should line up with the start of the second sentence in the line.", 26);
			} else if (k == ktype::star) {
				for (const haunted::ui::textbox::line_ptr &line: tb->lines) {
					DBG(line->continuation << "[" << std::string(*line) << "]");
				}
			} else {
				*tb += "Key: [" + std::string(k) + "]";
			}
		}
	}

	void maintest::test_expandobox(terminal &term) {
		using namespace haunted::ui;
		using namespace haunted::ui::boxes;

		term.cbreak();
		textbox   *tb  = new textbox();               tb->set_name("tb");
		textinput *ti  = new textinput();             ti->set_name("ti");
		label     *tlb = new label("Title",  false); tlb->set_name("tlb");
		label     *slb = new label("Status", false); slb->set_name("slb");

		expandobox *vexp = new expandobox(&term, term.get_position(), box_orientation::vertical, {
			{tlb, 1}, {tb, -1}, {slb, 1}, {ti, 1}
		});
		vexp->set_name("vexp").resize();

		term.redraw();
		ti->focus();

		tlb->set_colors(ansi::color::white, ansi::color::blue);
		slb->set_colors(ansi::color::white, ansi::color::blue);

		ti->listen(textinput::event::submit, [&](const superstring &sstr, int) {
			if (!sstr.empty()) {
				*tb += sstr;
				ti->clear();
			}
		});

		key k;
		while (term >> k) {
			if (k == key(ktype::c).ctrl())
				break;

			if (k == kmod::ctrl) {
				switch (k.type) {
					case ktype::F:  DBG("Focused: " << term.get_focused()->get_id());  break;
					case ktype::k:  dbgstream.clear().jump().flush();                  break;
					case ktype::l:  term.redraw();                                     break;

					case ktype::f: {
						tb->clear_lines();
						static std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
						for (int i = 0; i < tb->get_position().height - 1; ++i)
							*tb += std::string(1, alphabet[i % 26]);
						ti->focus();
						break;
					}

					default: term.send_key(k);
				}
			} else if (k == kmod::alt) {
				switch (k.type) {
					case ktype::c:  tb->clear_lines(); break;

					case ktype::l:
						ti->set_prefix(ti->empty()? "" : "[" + ti->str() + "] ");
						ti->clear();
						ti->jump_cursor();
						break;

					case ktype::s:
						if (!ti->empty()) {
							slb->set_text(*ti);
							ti->clear();
							ti->jump_cursor();
						}
						break;
					
					case ktype::t:
						if (!ti->empty()) {
							tlb->set_text(*ti);
							ti->clear();
							ti->jump_cursor();
						}
						break;

					default: term.send_key(k);
				}
			} else if (k.is_arrow() && k.mods == key::get_modset(kmod::shift)) {
				tb->on_key(key(k.type));
				ti->focus();
			} else {
				term.send_key(k);
			}
		}
	}

	/** Runs some tests for the CSI u functions. */
	void maintest::unittest_csiu(testing &unit) {
		using namespace std::string_literals;
		INFO(wrap("Testing CSI u validation.\n", ansi::style::bold));

		unit.check({
			{{"1;1u"s  },  true},
			{{"1;1a"s  }, false},
			{{";1u"s   }, false},
			{{"1;u"s   }, false},
			{{"4u"s    }, false},
			{{"1;u1"s  }, false},
			{{"1u;1"s  }, false},
			{{";1u"s   }, false},
			{{"u1;1"s  }, false},
			{{"42;0u"s },  true},
			{{"3;911u"s},  true},
			{{"5;5U"s  }, false},
		}, &csi::is_csiu, "is_csiu");

		// ansi::out << "\nTesting CSI u parsing.\n";
		// unit.check({
		// 	{"1;1u"s,    { 1,   1}},
		// 	{"42;0u"s,   {42,   0}},
		// 	{"3;911u"s,  { 3, 911}},
		// 	{"55;555u"s, {55, 555}},
		// 	{"1;1a"s,    {-1,  -2}},
		// 	{";1u"s,     {-1,  -1}},
		// 	{"1;u"s,     {-1,  -1}},
		// 	{"4u"s,      {-1,  -1}},
		// 	{"1;u1"s,    {-1,  -2}},
		// 	{"1u;1"s,    {-1,  -2}},
		// 	{";1u"s,     {-1,  -1}},
		// 	{"u1;1"s,    {-1,  -1}},
		// 	{"5;5U"s,    {-1,  -2}},
		// }, &parse_csi, "parse_csiu");

		ansi::out << ansi::endl;
	}

	void maintest::unittest_textbox(testing &unit) {
		using namespace haunted::ui;
		INFO(wrap("Testing haunted::ui::textbox.\n", ansi::style::bold));
		
		dummy_terminal dummy;

		boxes::simplebox wrapper(&dummy);
		wrapper.resize({0, 0, 20, 10});
		
		textbox *tb = new textbox(&wrapper, wrapper.get_position());

		std::shared_ptr<simpleline> t1 = std::make_shared<simpleline>("Hello", 4);
		std::shared_ptr<simpleline> t2 = std::make_shared<simpleline>("This line is longer than the control's width of 20 characters. Its continuation should align with the third word.", 10);
		std::shared_ptr<simpleline> t3 = std::make_shared<simpleline>("Another line.", 1);
		std::shared_ptr<simpleline> t4 = std::make_shared<simpleline>("This is another long line with a continuation of 0.");
		std::shared_ptr<simpleline> t5 = std::make_shared<simpleline>("Exactly 20 chars :^)", 0);

		*tb += *t1;
		*tb += *t2;
		*tb += *t3;
		*tb += *t3;
		*tb += *t4;
		*tb += *t5;

		int rows = tb->total_rows();

		unit.check(tb->line_rows(*t2), 11, "line_rows(" + ansi::wrap("t2", ansi::style::bold) + ")");
		unit.check(rows, 18, "total_rows()");

		unit.check({
			{{0},  {&*t1, 0}},
			{{1},  {&*t2, 0}},
			{{2},  {&*t2, 1}},
			{{3},  {&*t2, 2}},
			{{4},  {&*t2, 3}},
			{{5},  {&*t2, 4}},
			{{6},  {&*t2, 5}},
			{{7},  {&*t2, 6}},
			{{8},  {&*t2, 7}},
			{{9},  {&*t2, 8}},
			{{10}, {&*t2, 9}},
			{{11}, {&*t2, 10}},
			{{12}, {&*t3, 0}},
			{{13}, {&*t3, 0}},
			{{14}, {&*t4, 0}},
			{{15}, {&*t4, 1}},
			{{16}, {&*t4, 2}},
			{{17}, {&*t5, 0}},
		}, &textbox::line_at_row, tb, "line_at_row");

		unit.check("line_at_row(" + std::to_string(rows) + ")", typeid(std::out_of_range), "Invalid row index: " +
			std::to_string(rows), tb, &textbox::line_at_row, rows);

		unit.check({
			{{0, true}, "Hello               "},
			{{1, true}, "This line is longer "},
			{{2, true}, "          than the c"},
			{{3, true}, "          ontrol's w"},
			{{4, true}, "          idth of 20"},
			{{5, true}, "           character"},
			{{6, true}, "          s. Its con"},
			{{7, true}, "          tinuation "},
			{{8, true}, "          should ali"},
			{{9, true}, "          gn with th"},
			{{1, true}, ""},
		}, &textbox::text_at_row, tb, "text_at_row");

		INFO("Trying to scroll 10 lines down (current voffset is " << ansi::wrap(std::to_string(tb->voffset),
			ansi::style::bold) << ").");
		tb->vscroll(10);
		unit.check(tb->voffset, 8, "voffset");

		unit.check({
			{{0, true},  "          should ali"},
			{{1, true},  "          gn with th"},
			{{2, true},  "          e third wo"},
			{{3, true},  "          rd.       "},
			{{4, true},  "Another line.       "},
			{{5, true},  "Another line.       "},
			{{6, true},  "This is another long"},
			{{7, true},  " line with a continu"},
			{{8, true},  "ation of 0.         "},
			{{9, true},  "Exactly 20 chars :^)"},
			{{10, true}, ""},
		}, &textbox::text_at_row, tb, "textbox::text_at_row");

		unit.check(tb->next_row(), -1, "next_row()");
		INFO("Resetting textbox.");
		tb->clear_lines();
		unit.check("line_at_row(0)", typeid(std::out_of_range), "Invalid row index: 0", tb, &textbox::line_at_row, 0);
		unit.check(tb->voffset, 0, "voffset");
		unit.check(tb->total_rows(), 0, "total_rows()");
		// unit.check(tb->effective_voffset(), 0, "effective_voffset()");
		unit.check(tb->pos.height, 10, "pos.height");
		unit.check(tb->next_row(), 0, "next_row()");
		*tb += *t1;
		unit.check(tb->next_row(), 1, "next_row()");
		*tb += *t1;
		unit.check(tb->next_row(), 2, "next_row()");
		*tb += *t4;
		unit.check(tb->next_row(), 5, "next_row()");
		*tb += *t1;
		unit.check(tb->next_row(), 6, "next_row()");
		*tb += *t1;
		unit.check(tb->next_row(), 7, "next_row()");
		*tb += *t1;
		unit.check(tb->next_row(), 8, "next_row()");
		*tb += *t1;
		unit.check(tb->next_row(), 9, "next_row()");
		*tb += *t1;
		unit.check(tb->next_row(), -1, "next_row()");

		using namespace std::string_literals;
		unit.check(t1->text_at_row(tb->pos.width, 0), "Hello               "s, "t1.text_at_row(0)");
		unit.check(t2->text_at_row(tb->pos.width, 0), "This line is longer "s, "t2.text_at_row(0)");
		unit.check(t2->text_at_row(tb->pos.width, 1), "          than the c"s, "t2.text_at_row(1)");
		unit.check(t2->text_at_row(tb->pos.width, 2), "          ontrol's w"s, "t2.text_at_row(2)");
		unit.check(t2->text_at_row(tb->pos.width, 3), "          idth of 20"s, "t2.text_at_row(3)");
		unit.check(t2->text_at_row(tb->pos.width, 4), "           character"s, "t2.text_at_row(4)");
		unit.check(t2->text_at_row(tb->pos.width, 5), "          s. Its con"s, "t2.text_at_row(5)");
		unit.check(t2->text_at_row(tb->pos.width, 6), "          tinuation "s, "t2.text_at_row(6)");
		unit.check(t2->text_at_row(tb->pos.width, 7), "          should ali"s, "t2.text_at_row(7)");
		unit.check(t2->text_at_row(tb->pos.width, 8), "          gn with th"s, "t2.text_at_row(8)");

		ansi::out << ansi::endl;
	}

	void maintest::unittest_expandobox(testing &unit) {
		using namespace haunted::ui::boxes;
		INFO(wrap("Testing haunted::ui::boxes::expandobox.\n", ansi::style::bold));

		dummy_terminal dummy;

		simplebox wrapper(&dummy);
		wrapper.resize({10, 10, 500, 100});

		ui::textbox *tb1 = new ui::textbox(nullptr);
		ui::textbox *tb2 = new ui::textbox(nullptr);
		ui::textbox *tb3 = new ui::textbox(nullptr);
		ui::textbox *tb4 = new ui::textbox(nullptr);

		expandobox *expando = new expandobox(&wrapper, wrapper.get_position(), box_orientation::horizontal,
			{{tb1, 10}, {tb2, -1}});

		unit.check(wrapper.get_position(),  {10, 10, 500, 100}, "wrapper position");
		unit.check(expando->get_position(), {10, 10, 500, 100}, "expando position");
		unit.check(tb1->get_position(), {-1, -1, -1, -1}, "tb1 position");
		unit.check(tb2->get_position(), {-1, -1, -1, -1}, "tb2 position");
		INFO("Expanding children.");
		expando->resize();
		unit.check(tb1->get_position(), {10, 10, 10,  100}, "tb1 position");
		unit.check(tb2->get_position(), {20, 10, 490, 100}, "tb2 position");
		INFO("Adding tb3 and expanding.");
		(*expando += {tb3, 90}).resize();
		unit.check(tb1->get_position(), {10,  10, 10,  100}, "tb1 position");
		unit.check(tb2->get_position(), {20,  10, 400, 100}, "tb2 position");
		unit.check(tb3->get_position(), {420, 10, 90,  100}, "tb3 position");
		INFO("Adding tb4 and expanding.");
		(*expando += {tb4, -1}).resize();
		unit.check(tb1->get_position(), {10,  10, 10,  100}, "tb1 position");
		unit.check(tb2->get_position(), {20,  10, 200, 100}, "tb2 position");
		unit.check(tb3->get_position(), {220, 10, 90,  100}, "tb3 position");
		unit.check(tb4->get_position(), {310, 10, 200, 100}, "tb4 position");

		INFO("Reorienting horizontal → box_orientation::vertical.");
		expando->resize({10, 10, 100, 250});
		expando->set_orientation(box_orientation::vertical);
		unit.check(tb1->get_position(), {10, 10,  100, 10}, "tb1 position");
		unit.check(tb2->get_position(), {10, 20,  100, 75}, "tb2 position");
		unit.check(tb3->get_position(), {10, 95,  100, 90}, "tb3 position");
		unit.check(tb4->get_position(), {10, 185, 100, 75}, "tb4 position");

		std::string ansistr = "["_d + "00:00:00" + "] <"_d + "@kai" + "> "_d + "Hello there.";
		std::string stripped = ansi::strip(ansistr);
		unit.check(stripped, std::string("[00:00:00] <@kai> Hello there."), "ansi::strip()");
		unit.check(ansi::strip(ansi::substr(ansistr, 4, 18)), stripped.substr(4, 18), "ansi::strip(ansi::substr(4, 18))");
		unit.check(ansi::substr(ansistr, 4, 18), "00:00" + "] <"_d + "@kai" + "> "_d + "Hell", "ansi::substr(4, 18)");
		unit.check(ansi::length(ansistr), 30UL, "ansi::length()");
		unit.check(ansi::get_pos(ansistr, 0), 0UL, "ansi::get_pos(0)");
		unit.check(ansi::get_pos(ansistr, 1), 5UL, "ansi::get_pos(1)");
		unit.check(ansi::get_pos(ansistr, 2), 11UL, "ansi::get_pos(2)");
		unit.check(ansi::get_pos(ansistr, 3), 12UL, "ansi::get_pos(3)");
		const std::string str_extra = "This is another sentence to increase the length of the string so that it can be used for more unit tests.";
		ansistr += " " + str_extra;
		INFO("Appending \"" << str_extra << "\" to ansi_str.");
		unit.check(ansi::substr(ansistr, 0, 112), std::string("["_d + "00:00:00" + "] <"_d + "@kai" + "> "_d + "Hello there. This is another sentence to increase the length of the string so that it can be u"), "ansi::substr(0, 112)");

		ansi::out << ansi::endl;
	}
}


int main(int argc, char **argv) {
	using namespace haunted;
	
	const std::string arg = argc < 2? "" : argv[1];

	// terminal term(std::cin, ansi::out);
	// term.watch_size();

	haunted::tests::testing unit;

#define MKTERM terminal term(std::cin, ansi::out); term.watch_size();
	if (arg == "key") {
		mouse_mode mode = mouse_mode::none;
		if (2 < argc) {
			const std::string marg = argv[2];
			if (marg == "basic") {
				mode = mouse_mode::basic;
			} else if (marg == "normal") {
				mode = mouse_mode::normal;
			} else if (marg == "highlight") {
				mode = mouse_mode::highlight;
			} else if (marg == "motion") {
				mode = mouse_mode::motion;
			} else if (marg == "any") {
				mode = mouse_mode::any;
			} else if (marg == "none") {
				std::cout << "\e[?9l\e[?1000l\e[?1001l\e[?1002l\e[?1003l";
				return 0;
			} else {
				std::cerr << "Unknown mouse mode: " << marg << "\n";
				return 1;
			}

			std::cerr << "Enabled mouse mode " << marg << ".\n";
		}

		MKTERM

		if (mode != mouse_mode::none)
			term.mouse(mode);

		haunted::tests::maintest::test_key(term);
	} else if (arg == "input") { MKTERM
		haunted::tests::maintest::test_textinput(term);
	} else if (arg == "cursor") { MKTERM
		haunted::tests::maintest::test_cursor(term);
	} else if (arg == "margins") { MKTERM
		haunted::tests::maintest::test_margins(term);
	} else if (arg == "textbox") { MKTERM
		haunted::tests::maintest::test_textbox(term);
	} else if (arg == "expandobox") { MKTERM
		haunted::tests::maintest::test_expandobox(term);
	} else if (arg == "unitcsiu") {
		haunted::tests::maintest::unittest_csiu(unit);
	} else if (arg == "unittextbox") {
		haunted::tests::maintest::unittest_textbox(unit);
	} else if (arg == "unitexpandobox") {
		haunted::tests::maintest::unittest_expandobox(unit);
	} else if (arg == "unit") {
		ansi::out << ansi::endl;
		haunted::tests::maintest::unittest_csiu(unit);
		haunted::tests::maintest::unittest_textbox(unit);
		haunted::tests::maintest::unittest_expandobox(unit);
	} else {
		haunted::tests::maintest::unittest_textbox(unit);
	}
#undef MKTERM
}

