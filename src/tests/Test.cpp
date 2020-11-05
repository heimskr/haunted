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
#include "haunted/tests/Test.h"
#include "haunted/core/CSI.h"
#include "haunted/core/DummyTerminal.h"
#include "haunted/core/Key.h"
#include "haunted/core/Util.h"
#include "haunted/core/Terminal.h"
#include "haunted/ui/boxes/SimpleBox.h"
#include "haunted/ui/boxes/ExpandoBox.h"
#include "haunted/ui/Label.h"
#include "haunted/ui/Textbox.h"
#include "haunted/ui/TextInput.h"
#include "lib/ustring.h"

#ifdef NODEBUG
#define INFO(x)
#else
#define INFO(x) ansi::out << ansi::info << x << ansi::endl
#endif

namespace Haunted::Tests {
	std::pair<int, int> maintest::parse_csi(const std::string &input) {
		Haunted::CSI testcsi(input);
		return {testcsi.first, testcsi.second};
	}

	void maintest::test_textinput(Terminal &term) {
		Haunted::UI::TextInput *ti = new Haunted::UI::TextInput(&term);
		term.setRoot(ti);
		ti->focus();
		ti->resize({0, 0, term.getCols(), 1});
		term.cbreak();
		term.startInput();
	}

	void maintest::test_key(Terminal &term) {
		Key k;
		term.cbreak();
		while (term >> k) {
			if (k == KeyType::Mouse)
				continue;

			std::cout <<  "\r"  << std::setw(3) << std::left << std::setfill(' ')
			          << int(k) <<  " "  <<  k  << std::endl;
			if (k == '\\')
				break;
			else if (k == '.')
				term.raw = !term.raw;
		}
	}

	void maintest::test_cursor(Terminal &term) {
		term.outStream.clear();
		term.jump(180, 0);
		term << "0";
		term.jump(181, 1);
		term << "1";
		term.flush();
	}

	void maintest::test_margins(Terminal &term) {
		term.cbreak();
		term.outStream.clear();
		term.outStream.jump();
		const std::string spam = "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		for (int i = 0; i < 40; ++i) {
			std::cout << spam.substr(i % spam.length(), std::string::npos) << spam.substr(0, i % spam.length()) << "\n";
		}

		sleep(1); std::cerr << "Setting margins.\n";
		term.enableHmargins();
		term.margins(2, 10, 5, 15);
		
		sleep(1); std::cerr << "Scrolling up.\n";
		term.outStream.scroll_up(2);
		
		sleep(1); std::cerr << "Scrolling down.\n";
		term.outStream.scroll_down(1);

		sleep(2); std::cerr << "Jumping and printing.\n";
		term.outStream.jump(5, 2);
		term << "Hello";
		term.flush();

		sleep(2); std::cerr << "Setting origin, jumping and printing.\n";
		term.setOrigin();
		term.outStream.jump();
		term << "Hi :)";
		term.flush();

		sleep(2); std::cerr << "Clearing line.\n";
		term.outStream.clear_line();

		sleep(1); std::cerr << "Clearing all.\n";
		term.outStream.clear();

		sleep(1); std::cerr << "Resetting margins.\n";
		term.margins();

		sleep(5); std::cerr << "Scrolling up.\n";
		term.outStream.scroll_up(2);

		sleep(5); std::cerr << "Done.\n";
		term.disableHmargins();
	}

	void maintest::test_textbox(Terminal &term) {
		using Haunted::UI::TextLine, Haunted::UI::SimpleLine;

		term.cbreak();
		Haunted::UI::Textbox *tb = new Haunted::UI::Textbox(&term);
		*tb += "hi";
		*tb += "what's up :)";
		*tb += "third line.";
		term.draw();
		Key k;
		while (term >> k) {
			if (k == '\\')
				break;
			if (k == KeyType::UpArrow) {
				tb->vscroll(-1);
				tb->draw();
			} else if (k == KeyType::DownArrow) {
				tb->vscroll(1);
				tb->draw();
			} else if (k == KeyType::LeftArrow) {
				tb->setVoffset(-1);
				tb->draw();
			} else if (k == KeyType::Hash) {
				*tb += SimpleLine("This is a very long line. Its purpose is to test the continuation of lines in a textbox. Its continuation value is set to 26, so the wrapped text should line up with the start of the second sentence in the line.", 26);
			} else if (k == KeyType::Star) {
				for (const Haunted::UI::Textbox::LinePtr &line: tb->lines) {
					DBG(line->getContinuation() << "[" << std::string(*line) << "]");
				}
			} else {
				*tb += "Key: [" + std::string(k) + "]";
			}
		}
	}

	void maintest::test_expandobox(Terminal &term) {
		using namespace Haunted::UI;
		using namespace Haunted::UI::Boxes;

		term.cbreak();
		Textbox   *tb  = new Textbox();               tb->setName("tb");
		TextInput *ti  = new TextInput();             ti->setName("ti");
		Label     *tlb = new Label("Title",  false); tlb->setName("tlb");
		Label     *slb = new Label("Status", false); slb->setName("slb");

		ExpandoBox *vexp = new ExpandoBox(&term, term.getPosition(), BoxOrientation::Vertical, {
			{tlb, 1}, {tb, -1}, {slb, 1}, {ti, 1}
		});
		vexp->setName("vexp").resize();

		term.redraw();
		ti->focus();

		tlb->setColors(ansi::color::white, ansi::color::blue);
		slb->setColors(ansi::color::white, ansi::color::blue);

		ti->listen(TextInput::Event::Submit, [&](const ustring &sstr, int) {
			if (!sstr.empty()) {
				*tb += sstr;
				ti->clear();
			}
		});

		Key k;
		while (term >> k) {
			if (k == Key(KeyType::c).ctrl())
				break;

			if (k == KeyMod::Ctrl) {
				switch (k.type) {
					case KeyType::F:  DBG("Focused: " << term.getFocused()->getID());  break;
					case KeyType::k:  dbgstream.clear().jump().flush();                  break;
					case KeyType::l:  term.redraw();                                     break;

					case KeyType::f: {
						tb->clearLines();
						static std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
						for (int i = 0; i < tb->getPosition().height - 1; ++i)
							*tb += std::string(1, alphabet[i % 26]);
						ti->focus();
						break;
					}

					default: term.sendKey(k);
				}
			} else if (k == KeyMod::Alt) {
				switch (k.type) {
					case KeyType::c:  tb->clearLines(); break;

					case KeyType::l:
						ti->setPrefix(ti->empty()? "" : "[" + ti->str() + "] ");
						ti->clear();
						ti->jumpCursor();
						break;

					case KeyType::s:
						if (!ti->empty()) {
							slb->setText(*ti);
							ti->clear();
							ti->jumpCursor();
						}
						break;
					
					case KeyType::t:
						if (!ti->empty()) {
							tlb->setText(*ti);
							ti->clear();
							ti->jumpCursor();
						}
						break;

					default: term.sendKey(k);
				}
			} else if (k.isArrow() && k.mods == Key::getModSet(KeyMod::Shift)) {
				tb->onKey(Key(k.type));
				ti->focus();
			} else {
				term.sendKey(k);
			}
		}
	}

	/** Runs some tests for the CSI u functions. */
	void maintest::unittest_csiu(Testing &unit) {
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
		}, &CSI::isCSIu, "is_csiu");

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

	void maintest::unittest_textbox(Testing &unit) {
		using namespace Haunted::UI;
		INFO(wrap("Testing Haunted::UI::Textbox.\n", ansi::style::bold));
		
		DummyTerminal dummy;

		Boxes::SimpleBox wrapper(&dummy);
		wrapper.resize({0, 0, 20, 10});
		
		Textbox *tb = new Textbox(&wrapper, wrapper.getPosition());

		std::shared_ptr<SimpleLine> t1 = std::make_shared<SimpleLine>("Hello", 4);
		std::shared_ptr<SimpleLine> t2 = std::make_shared<SimpleLine>("This line is longer than the control's width of 20 characters. Its continuation should align with the third word.", 10);
		std::shared_ptr<SimpleLine> t3 = std::make_shared<SimpleLine>("Another line.", 1);
		std::shared_ptr<SimpleLine> t4 = std::make_shared<SimpleLine>("This is another long line with a continuation of 0.");
		std::shared_ptr<SimpleLine> t5 = std::make_shared<SimpleLine>("Exactly 20 chars :^)", 0);

		*tb += *t1;
		*tb += *t2;
		*tb += *t3;
		*tb += *t3;
		*tb += *t4;
		*tb += *t5;

		int rows = tb->totalRows();

		unit.check(tb->lineRows(*t2), 11, "lineRows(" + ansi::wrap("t2", ansi::style::bold) + ")");
		unit.check(rows, 18, "totalRows()");

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
		}, &Textbox::lineAtRow, tb, "lineAtRow");

		unit.check("lineAtRow(" + std::to_string(rows) + ")", typeid(std::out_of_range), "Invalid row index: " +
			std::to_string(rows), tb, &Textbox::lineAtRow, rows);

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
			{{10, true}, ""},
		}, &Textbox::textAtRow, tb, "textAtRow");

		INFO("Trying to scroll 8 lines down (current voffset is " << ansi::wrap(std::to_string(tb->voffset),
			ansi::style::bold) << ").");
		tb->vscroll(8);
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
		}, &Textbox::textAtRow, tb, "textbox::textAtRow");

		unit.check(tb->nextRow(), -1, "nextRow()");
		INFO("Resetting textbox.");
		tb->clearLines();
		unit.check("lineAtRow(0)", typeid(std::out_of_range), "Invalid row index: 0", tb, &Textbox::lineAtRow, 0);
		unit.check(tb->voffset, 0, "voffset");
		unit.check(tb->totalRows(), 0, "totalRows()");
		// unit.check(tb->effective_voffset(), 0, "effective_voffset()");
		unit.check(tb->position.height, 10, "pos.height");
		unit.check(tb->nextRow(), 0, "nextRow()");
		*tb += *t1;
		unit.check(tb->nextRow(), 1, "nextRow()");
		*tb += *t1;
		unit.check(tb->nextRow(), 2, "nextRow()");
		*tb += *t4;
		unit.check(tb->nextRow(), 5, "nextRow()");
		*tb += *t1;
		unit.check(tb->nextRow(), 6, "nextRow()");
		*tb += *t1;
		unit.check(tb->nextRow(), 7, "nextRow()");
		*tb += *t1;
		unit.check(tb->nextRow(), 8, "nextRow()");
		*tb += *t1;
		unit.check(tb->nextRow(), 9, "nextRow()");
		*tb += *t1;
		unit.check(tb->nextRow(), -1, "nextRow()");

		using namespace std::string_literals;
		unit.check(t1->textAtRow(tb->position.width, 0), "Hello               "s, "t1.textAtRow(0)");
		unit.check(t2->textAtRow(tb->position.width, 0), "This line is longer "s, "t2.textAtRow(0)");
		unit.check(t2->textAtRow(tb->position.width, 1), "          than the c"s, "t2.textAtRow(1)");
		unit.check(t2->textAtRow(tb->position.width, 2), "          ontrol's w"s, "t2.textAtRow(2)");
		unit.check(t2->textAtRow(tb->position.width, 3), "          idth of 20"s, "t2.textAtRow(3)");
		unit.check(t2->textAtRow(tb->position.width, 4), "           character"s, "t2.textAtRow(4)");
		unit.check(t2->textAtRow(tb->position.width, 5), "          s. Its con"s, "t2.textAtRow(5)");
		unit.check(t2->textAtRow(tb->position.width, 6), "          tinuation "s, "t2.textAtRow(6)");
		unit.check(t2->textAtRow(tb->position.width, 7), "          should ali"s, "t2.textAtRow(7)");
		unit.check(t2->textAtRow(tb->position.width, 8), "          gn with th"s, "t2.textAtRow(8)");

		ansi::out << ansi::endl;
	}

	void maintest::unittest_expandobox(Testing &unit) {
		using namespace Haunted::UI::Boxes;
		INFO(wrap("Testing Haunted::UI::Boxes::expandobox.\n", ansi::style::bold));

		DummyTerminal dummy;

		SimpleBox wrapper(&dummy);
		wrapper.resize({10, 10, 500, 100});

		UI::Textbox *tb1 = new UI::Textbox(nullptr);
		UI::Textbox *tb2 = new UI::Textbox(nullptr);
		UI::Textbox *tb3 = new UI::Textbox(nullptr);
		UI::Textbox *tb4 = new UI::Textbox(nullptr);

		ExpandoBox *expando = new ExpandoBox(&wrapper, wrapper.getPosition(), BoxOrientation::Horizontal,
			{{tb1, 10}, {tb2, -1}});

		unit.check(wrapper.getPosition(),  {10, 10, 500, 100}, "wrapper position");
		unit.check(expando->getPosition(), {10, 10, 500, 100}, "expando position");
		unit.check(tb1->getPosition(), {-1, -1, -1, -1}, "tb1 position");
		unit.check(tb2->getPosition(), {-1, -1, -1, -1}, "tb2 position");
		INFO("Expanding children.");
		expando->resize();
		unit.check(tb1->getPosition(), {10, 10, 10,  100}, "tb1 position");
		unit.check(tb2->getPosition(), {20, 10, 490, 100}, "tb2 position");
		INFO("Adding tb3 and expanding.");
		(*expando += {tb3, 90}).resize();
		unit.check(tb1->getPosition(), {10,  10, 10,  100}, "tb1 position");
		unit.check(tb2->getPosition(), {20,  10, 400, 100}, "tb2 position");
		unit.check(tb3->getPosition(), {420, 10, 90,  100}, "tb3 position");
		INFO("Adding tb4 and expanding.");
		(*expando += {tb4, -1}).resize();
		unit.check(tb1->getPosition(), {10,  10, 10,  100}, "tb1 position");
		unit.check(tb2->getPosition(), {20,  10, 200, 100}, "tb2 position");
		unit.check(tb3->getPosition(), {220, 10, 90,  100}, "tb3 position");
		unit.check(tb4->getPosition(), {310, 10, 200, 100}, "tb4 position");

		INFO("Reorienting horizontal → BoxOrientation::Vertical.");
		expando->resize({10, 10, 100, 250});
		expando->setOrientation(BoxOrientation::Vertical);
		unit.check(tb1->getPosition(), {10, 10,  100, 10}, "tb1 position");
		unit.check(tb2->getPosition(), {10, 20,  100, 75}, "tb2 position");
		unit.check(tb3->getPosition(), {10, 95,  100, 90}, "tb3 position");
		unit.check(tb4->getPosition(), {10, 185, 100, 75}, "tb4 position");

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

	void maintest::unittest_ustring(Testing &unit) {
		std::string example = "foo🎉🇩🇪👮🏻‍♂️bar👨‍👨‍👧‍👦";
		ustring uexample(example);
		std::string pieces[] = {"f", "o", "o", "🎉", "🇩🇪", "👮🏻‍♂️", "b", "a", "r", "👨‍👨‍👧‍👦"};
		int i = 0;
#ifndef ENABLE_ICU
		// for (char ch: uexample) {
		// 	unit.check(ch, pieces[i].front(), "pieces[" + std::to_string(i) + "]");
		// 	++i;
		// }
#else
		for (const std::string &piece: uexample) {
			unit.check(piece, pieces[i], "pieces[" + std::to_string(i) + "]");
			++i;
		}
#endif

		unit.check(uexample.substr(0, 1), "f",  "substr(0, 1)");
		unit.check(uexample.substr(1, 2), "oo", "substr(1, 2)");
		unit.check(uexample.substr(3, 1), "🎉", "substr(3, 1)");
		unit.check(uexample.length(), 10UL, "length()");
		unit.check(uexample.substr(uexample.length(), 10), "", "substr(length(), 10)");
		unit.check(uexample.substr(uexample.length() - 1, 10), "👨‍👨‍👧‍👦", "substr(length() - 1, 10)");
		unit.check("substr(length() + 1, 1)", typeid(std::out_of_range), "Invalid index: 11 (length is 10)", &uexample,
			&ustring::substr, uexample.length() + 1, 1UL);
#ifdef ENABLE_ICU
		unit.check(uexample[5], "👮🏻‍♂️", "uexample[5]");
		unit.check(uexample.at(5), "👮🏻‍♂️", "at(5)");
#endif
		ansi::out << ansi::info << "Inserting " << "\""_d << "baz"_b << "\""_d << " at index 4." << ansi::endl;
		uexample.insert(4, "baz");
		unit.check(uexample, "foo🎉baz🇩🇪👮🏻‍♂️bar👨‍👨‍👧‍👦", "uexample");
#ifdef ENABLE_ICU
		unit.check(uexample.widthUntil(11), 14UL, "uexample.widthUntil(11)");
		unit.check(uexample.widthUntil(13), 17UL, "uexample.widthUntil(13)");
#endif
		unit.check(uexample.substr(7, 2), "🇩🇪👮🏻‍♂️", "substr(7, 2)");
		unit.check(uexample.length(), 13UL, "length()");
		i = 0;
#ifdef ENABLE_ICU
		for (size_t len: {1, 1, 1, 2, 1, 1, 1, 2, 2, 1, 1, 1, 2}) {
			unit.check(uexample.width_at(i), len, "width_at(" + std::to_string(i) + ")");
			++i;
		}
#endif
		ansi::out << ansi::info << "Erasing 3 characters at index 6." << ansi::endl;
		uexample.erase(6, 3);
		unit.check(uexample, "foo🎉babar👨‍👨‍👧‍👦", "uexample");
		unit.check(uexample.length(), 10UL, "length()");
		ansi::out << ansi::info << "Erasing 10 characters at index 7." << ansi::endl;
		uexample.erase(7, 10);
		unit.check(uexample, "foo🎉bab", "uexample");
		unit.check(uexample.length(), 7UL, "length()");
		ansi::out << ansi::info << "Erasing 0 characters at index 0." << ansi::endl;
		uexample.erase(0, 0);
		unit.check(uexample, "foo🎉bab", "uexample");
		unit.check(uexample.length(), 7UL, "length()");

		ansi::out << ansi::endl;
	}
}


int main(int argc, char **argv) {
	using namespace Haunted;
	
	const std::string arg = argc < 2? "" : argv[1];

	// terminal term(std::cin, ansi::out);
	// term.watch_size();

	Haunted::Tests::Testing unit;

#define MKTERM Terminal term(std::cin, ansi::out); term.watchSize();
	if (arg == "key") {
		MouseMode mode = MouseMode::None;
		if (2 < argc) {
			const std::string marg = argv[2];
			if (marg == "basic") {
				mode = MouseMode::Basic;
			} else if (marg == "normal") {
				mode = MouseMode::Normal;
			} else if (marg == "highlight") {
				mode = MouseMode::Highlight;
			} else if (marg == "motion") {
				mode = MouseMode::Motion;
			} else if (marg == "any") {
				mode = MouseMode::Any;
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

		if (mode != MouseMode::None)
			term.mouse(mode);

		Haunted::Tests::maintest::test_key(term);
	} else if (arg == "input") { MKTERM
		Haunted::Tests::maintest::test_textinput(term);
	} else if (arg == "cursor") { MKTERM
		Haunted::Tests::maintest::test_cursor(term);
	} else if (arg == "margins") { MKTERM
		Haunted::Tests::maintest::test_margins(term);
	} else if (arg == "textbox") { MKTERM
		Haunted::Tests::maintest::test_textbox(term);
	} else if (arg == "expandobox") { MKTERM
		Haunted::Tests::maintest::test_expandobox(term);
	} else if (arg == "unitcsiu") {
		Haunted::Tests::maintest::unittest_csiu(unit);
	} else if (arg == "unittextbox") {
		Haunted::Tests::maintest::unittest_textbox(unit);
	} else if (arg == "unitexpandobox") {
		Haunted::Tests::maintest::unittest_expandobox(unit);
	} else if (arg == "unitustring") {
		Haunted::Tests::maintest::unittest_ustring(unit);
	} else if (arg == "unit") {
		ansi::out << ansi::endl;
		Haunted::Tests::maintest::unittest_csiu(unit);
		Haunted::Tests::maintest::unittest_textbox(unit);
		Haunted::Tests::maintest::unittest_expandobox(unit);
		Haunted::Tests::maintest::unittest_ustring(unit);
	} else {
		Haunted::Tests::maintest::unittest_textbox(unit);
	}
#undef MKTERM
}
