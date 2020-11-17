#include <deque>
#include <iostream>
#include <stdexcept>
#include <string>

#include <csignal>
#include <sys/ioctl.h>
#include <unistd.h>

#include "haunted/core/CSI.h"
#include "haunted/core/Key.h"
#include "haunted/core/Terminal.h"
#include "haunted/core/Util.h"
#include "haunted/ui/Child.h"
#include "haunted/ui/Control.h"

#include "lib/formicine/futil.h"

namespace Haunted {
	using uchar = unsigned char;

	std::vector<Terminal *> Terminal::winchTargets {};

	Terminal::Terminal(std::istream &inStream, ansi::ansistream &outStream):
	inStream(inStream), outStream(outStream), colors(&outStream, &outputMutex) {
		original = attrs = getattr();
		winsize size;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
		rows = size.ws_row;
		cols = size.ws_col;
	}

	Terminal::~Terminal() {
		if (!suppressOutput) {
			outStream.reset_colors();
			outStream.clear();
			reset();
			join();
			jump(0, 0);
		}

		delete root;
	}


// Private static methods


	void Terminal::winchHandler(int) {
		winsize new_size;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &new_size);
		for (Terminal *terminal: winchTargets)
			terminal->winch(new_size.ws_row, new_size.ws_col);
	}

	termios Terminal::getattr() {
		termios out;
		int result;
		if ((result = tcgetattr(STDIN_FILENO, &out)) < 0)
			throw std::runtime_error("tcgetattr returned " + std::to_string(result));

		return out;
	}

	void Terminal::setattr(const termios &new_attrs) {
		int result;
		if ((result = tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_attrs)) < 0)
			throw std::runtime_error("tcsetattr returned " + std::to_string(result));
	}


// Private instance methods


	void Terminal::apply() {
		setattr(attrs);
	}

	void Terminal::reset() {
		mouse(MouseMode::None);
		setattr(original);
		attrs = original;
	}

	void Terminal::workInput() {
		Key key;
		// Sometimes, calling cbreak() once doesn't seem to properly set all the flags (e.g., arrow keys produce strings
		// like "^[[C"). Calling it twice appears to work, but it's not pretty.
		cbreak();
		cbreak();
		while (alive) {
			*this >> key;
			if (key == Key(KeyType::c, KeyMod::Ctrl) && (!onInterrupt || onInterrupt()))
				break;
			sendKey(key);
		}
	}

	void Terminal::winch(int new_rows, int new_cols) {
		bool changed = rows != new_rows || cols != new_cols;
		rows = new_rows;
		cols = new_cols;
		if (changed) {
			std::unique_lock<std::mutex> lock(winchMutex);
			redraw();
		}
	}


// Public instance methods


	void Terminal::cbreak() {
		attrs.c_lflag &= ~(ECHO | ICANON | ISIG);
		attrs.c_iflag &= ~IXON;
		apply();
	}

	void Terminal::watchSize() {
		if (winchTargets.empty())
			std::signal(SIGWINCH, &Terminal::winchHandler);
		winchTargets.push_back(this);
	}

	void Terminal::redraw() {
		if (root) {
			colors.reset();
			outStream.clear().jump();
			root->resize({0, 0, cols, rows});
		}
	}

	void Terminal::setRoot(UI::Control *new_root, bool delete_old) {
		if (root != new_root) {
			if (delete_old)
				delete root;
			root = new_root;
			redraw();
		}
	}

	void Terminal::draw() {
		if (root)
			root->draw();
	}

	void Terminal::resetColors() {
		colors.reset();
	}

	UI::InputHandler * Terminal::sendKey(const Key &key) {
		// If the root is null, there are no controls and nothing to send key presses to.
		if (root == nullptr)
			return nullptr;

		UI::Control *control = getFocused();

		if (!control)
			throw std::runtime_error("Focused control is null");

		if (control->onKey(key)) {
			if (keyPostlistener)
				keyPostlistener(key);
			return control;
		}

		UI::Container *ptr = control->getParent();
		
		// Keep trying on_key, going up to the root as long as we keep getting false. If we're at the root and on_key
		// still returns false, let the terminal itself handle the keypress as a last resort.
		while (ptr && !ptr->onKey(key)) {
			if (dynamic_cast<UI::Control *>(ptr) == root) {
				onKey(key);
				if (keyPostlistener)
					keyPostlistener(key);
				return this;
			}

			if (UI::Child *cptr = dynamic_cast<UI::Child *>(ptr)) {
				ptr = cptr->getParent();
			} else {
				if (keyPostlistener)
					keyPostlistener(key);
				return nullptr;
			}
		}

		if (keyPostlistener)
			keyPostlistener(key);

		return ptr;
	}

	UI::InputHandler * Terminal::sendMouse(const MouseReport &report) {
		UI::Control *control = childAtOffset(report.x, report.y);

		if (control == nullptr) {
			DBG(report.str() << " nullptr"_d);
			return nullptr;
		}

		if (control->onMouse(report)) {
			if (mousePostlistener)
				mousePostlistener(report);
			return control;
		}

		UI::Container *ptr = control->getParent();

		while (ptr && !ptr->onMouse(report)) {
			if (dynamic_cast<UI::Control *>(ptr) == root) {
				onMouse(report);
				if (mousePostlistener)
					mousePostlistener(report);
				return this;
			}

			if (UI::Child *cptr = dynamic_cast<UI::Child *>(ptr)) {
				ptr = cptr->getParent();
			} else {
				if (mousePostlistener)
					mousePostlistener(report);
				return nullptr;
			}
		}

		if (mousePostlistener)
			mousePostlistener(report);

		return ptr;
	}

	bool Terminal::onKey(const Key &key) {
		if (key == KeyMod::Ctrl) {
			switch (key.type) {
				case KeyType::l:
					redraw();
					break;
				case KeyType::y:
					debugTree();
					break;
				default:
					return false;
			}

			return true;
		}

		return false;
	}

	void Terminal::startInput() {
		inputThread = std::thread(&Terminal::workInput, this);
	}

	void Terminal::join() {
		if (inputThread.joinable())
			inputThread.join();
	}

	void Terminal::flush() {
		outStream.flush();
	}

	void Terminal::focus(UI::Control *to_focus) {
		focused = to_focus;
	}

	UI::Control * Terminal::getFocused() {
		if (focused)
			return focused;
		focused = root;
		return root;
	}

	bool Terminal::addChild(UI::Control *) {
		return false;
	}

	bool Terminal::hasFocus(const UI::Control *control) const {
		return focused == control;
	}

	Position Terminal::getPosition() const {
		return {0, 0, cols, rows};
	}

	UI::Control * Terminal::childAtOffset(int x, int y) const {
		UI::Container *container = dynamic_cast<UI::Container *>(root);
		while (container != nullptr) {
			UI::Control *control = container->childAtOffset(x, y);
			if (control == nullptr)
				return nullptr;
			container = dynamic_cast<UI::Container *>(control);
			if (container == nullptr)
				return control;
		}

		return nullptr;
	}

	void Terminal::jumpToFocused() {
		if (focused)
			focused->jumpFocus();
	}

	void Terminal::jump(int x, int y) {
		std::unique_lock<std::mutex> uniq(outputMutex);
		outStream.jump(x, y);
	}

	void Terminal::mouse(MouseMode mode) {
		std::unique_lock<std::mutex> uniq(outputMutex);
		if (mode == MouseMode::None) {
			if (mmode != mode) {
				outStream << "\e[?" << std::to_string(int(mmode)) << ";1006l";
				mmode = mode;
			}

			return;
		}

		if (mode != mmode) {
			if (mmode != MouseMode::None)
				outStream << "\e[?" << std::to_string(int(mmode)) << "l";
			outStream << "\e[?" << std::to_string(int(mode)) << ";1006h";
			mmode = mode;
		}
	}

	void Terminal::vscroll(int rows) {
		std::unique_lock<std::mutex> uniq(outputMutex);
		if (0 < rows) {
			outStream.scroll_down(rows);
		} else if (rows < 0) {
			outStream.scroll_up(-rows);
		}
	}

	void Terminal::hmargins(size_t left, size_t right) {
		std::unique_lock<std::mutex> uniq(outputMutex);
		outStream.hmargins(left, right);
	}

	void Terminal::hmargins() {
		std::unique_lock<std::mutex> uniq(outputMutex);
		outStream.hmargins();
	}

	void Terminal::vmargins(size_t top, size_t bottom) {
		std::unique_lock<std::mutex> uniq(outputMutex);
		outStream.vmargins(top, bottom);
	}

	void Terminal::vmargins() {
		std::unique_lock<std::mutex> uniq(outputMutex);
		outStream.vmargins();
	}

	void Terminal::margins(size_t top, size_t bottom, size_t left, size_t right) {
		vmargins(top, bottom);
		hmargins(left, right);
	}

	void Terminal::margins() {
		hmargins();
		vmargins();
	}

	void Terminal::enableHmargins() { // DECLRMM: Left Right Margin Mode
		std::unique_lock<std::mutex> uniq(outputMutex);
		outStream.enable_hmargins();
	}

	void Terminal::disableHmargins() {
		std::unique_lock<std::mutex> uniq(outputMutex);
		outStream.disable_hmargins();
	}

	void Terminal::setOrigin() {
		std::unique_lock<std::mutex> uniq(outputMutex);
		outStream.set_origin();
	}

	void Terminal::resetOrigin() {
		std::unique_lock<std::mutex> uniq(outputMutex);
		outStream.reset_origin();
	}

	std::unique_lock<std::recursive_mutex> Terminal::lockRender() {
		return std::unique_lock<std::recursive_mutex>(renderMutex);
	}


// Public operators


	Terminal::operator bool() const {
		return bool(inStream);
	}

	Terminal & Terminal::operator>>(int &ch) {
		if (int c = inStream.get())
			ch = c;
		return *this;
	}

	Terminal & Terminal::operator>>(char &ch) {
		char c = 0;
		if (inStream.get(c))
			ch = c;
		return *this;
	}

	Terminal & Terminal::operator>>(Key &key) {
		// If we receive an escape followed by a [ and another escape, we return Alt+[ after receiving the second
		// escape, but this discards the second escape. To make up for this, we use a static bool to indicate that this
		// weird sequence has occurred.
		static bool partial_escape = false;

		char c;
		if (raw) {
			*this >> c;
			key = c;
			return *this;
		}

		key = 0;

		if (!(*this >> c))
			return *this;

		// It's important to reset the partial_escape flag. Resetting it right after reading it prevents me from having
		// to insert a reset before every return statement.
		bool escape = partial_escape || c == uchar(KeyType::Escape);
		partial_escape = false;

		if (escape) {
			// If we read an escape byte, that means something interesting is about to happen.
			partial_escape = true; // ???

			if (!(*this >> c))
				return *this;

			if (c == uchar(KeyType::Escape)) {
				// We can't tell the difference between an actual press of the escape key and the beginning of a CSI.
				// Perhaps it would be possible with the use of some timing trickery, but I don't consider that
				// necessary right now (YAGNI!). Instead, the user will have to press the escape key twice.
				key = {c, KeyMod::None};
				return *this;
			} else if (c == uchar(KeyType::OpenSquare)) {
				if (!(*this >> c))
					return *this;

				switch (c) {
					// To input an actual Alt+[, the user has to press the [ key again. Otherwise, we wouldn't be able
					// to tell the difference between an actual Alt+[ and the beginning of a CSI.
					case '[':
						key = {c, KeyMod::Alt};
						return *this;

					// If there's another escape immediately after "^[", we'll assume the user typed an actual Alt+[ and
					// then input another escape sequence.
					case int(KeyType::Escape):
						partial_escape = false;
						key = {'[', KeyMod::Alt};
						return *this;

					// If the first character after the [ is A, B, C or D, it's an arrow key.
					case 'A': key = KeyType::UpArrow;    return *this;
					case 'B': key = KeyType::DownArrow;  return *this;
					case 'C': key = KeyType::RightArrow; return *this;
					case 'D': key = KeyType::LeftArrow;  return *this;
				}

				// At this point, we haven't yet determined what the input is. A CSI sequence ends with a character in
				// the range [0x40, 0x7e]. Let's read until we encounter one.
				static std::string buffer;
				buffer = c;

				while (!Util::isFinalchar(c)) {
					if (!(*this >> c))
						return *this;

					buffer += c;
				}

				const char back = buffer.back();
				if (back == 'M' || back == 'm') {
					if (buffer.front() != '<') {
						DBG("Unrecognized sequence: \"" << buffer << "\"");
						throw std::invalid_argument("Unrecognized sequence");
					}

					MouseReport report(buffer);

					if (report.action == MouseAction::Down) {
						dragging = true;
						dragButton = report.button;
					} else if (report.action == MouseAction::Up) {
						dragging = false;
					}

					if (dragging && report.action == MouseAction::Move) {
						report.action = MouseAction::Drag;
						report.button = dragButton;
					}

					sendMouse(report);

					key = KeyType::Mouse;
					return *this;
				}

				const CSI parsed = buffer;

				// Sometimes, getKey() returns keys with modifiers already set. For example, ^[Z represents shift+tab.
				// If these modifiers are already set, then modifiers weren't specified the CSI u way and we shouldn't
				// change them.
				key = parsed.getKey();
				if (key.mods.none())
					key = Key(key.type, ModSet((parsed.second - 1) & 7));

				return *this;
			}

			key = {c, KeyMod::Alt};
		} else if (c == 9) {
			key = {KeyType::Tab};
		} else if (c == 10) {
			key = {KeyType::Enter};
		} else if (c == 13) {
			key = {KeyType::CarriageReturn};
		} else if (0 < c && c < 27) {
			// 1..26 corresponds to ^a..^z.
			key = {uchar(KeyType::a) + c - 1, KeyMod::Ctrl};
		} else {
			key = c;
		}

		return *this;
	}

	void Terminal::debugTree() {
		ansi::ansistream &dbg = Haunted::dbgstream;
		dbg << "terminal"_b << ansi::endl;

		if (root) {
			std::deque<std::pair<int, UI::Control *>> queue {{1, root}};

			int depth;
			UI::Control *control;

			while (!queue.empty()) {
				std::tie(depth, control) = queue.back();
				queue.pop_back();

				const Haunted::Position &pos = control->getPosition();
				int width = pos.width, height = pos.height, top = pos.top, left = pos.left;

				dbg << ansi::color::gray << std::string(depth * 2, ' ') << ansi::action::reset
					<< control->getID() << (control->hasFocus()? " *"_d : "");
				dbg.jump(25, -1).save()        << "("_d << left << ","_d;
				dbg.restore().right(6)         << top << ") "_d;
				dbg.restore().right(10).save() << width;
				dbg.restore().right(3)         << " × "_d << height << ansi::endl;
				if (UI::Container *cont = dynamic_cast<UI::Container *>(control))
					for (UI::Control *child: cont->getChildren())
						queue.push_back({depth + 1, child});
			}
		}
	}
}
