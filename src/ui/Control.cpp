#include <sstream>

#include "haunted/core/Terminal.h"
#include "haunted/core/Util.h"
#include "haunted/ui/Control.h"

#include "lib/formicine/ansi.h"

namespace Haunted::UI {
	Control::Control(Container *parent_, const Haunted::Position &position_):
	Child(parent_), terminal(nullptr), position(position_) {
		if (parent_)
			terminal = parent_->getTerminal();
	}

	Control::Control(Container *parent_, Terminal *terminal_):
		Child(parent_), terminal(terminal_) {}

	Control::Control(Container *parent_):
		Control(parent_, parent_ == nullptr? nullptr : parent_->getTerminal()) {}

	Control::~Control() = default;


// Protected instance methods


	bool Control::tryMargins(std::function<void()> fn) {
		const bool should_reset_margins = !inMargins;

		if (should_reset_margins)
			setMargins();

		fn();

		if (should_reset_margins)
			resetMargins();

		return should_reset_margins;
	}


// Public instance methods


	void Control::resize(const Haunted::Position &new_pos) {
		// It's up to the caller of resize() to also call draw().
		position = new_pos;
	}

	std::string Control::getID(bool pad) const {
		std::stringstream ss;
		if (name.empty()) {
			std::string demangled = Util::demangleObject(*this);
			if (pad)
				ss << std::setw(10);
			ss << demangled.substr(demangled.find_last_of(':') + 1) << "\e[2m|\e[0;33m" << this << "\e[39m";
		} else {
			
			// ss << ansi::get_fg(static_cast<ansi::color>(reinterpret_cast<size_t>(this) & 0x10));
			ansi::color c = static_cast<ansi::color>((reinterpret_cast<size_t>(this) >> 6) & 15);
			if (c == ansi::color::black) {
				c = ansi::color::green;
			} else if (c == ansi::color::normal) {
				c = ansi::color::red;
			}

			ss << ansi::get_fg(c);
			if (pad)
				ss << std::setw(10);
			ss << name << ansi::get_fg(ansi::color::normal);
		}

		return ss.str();
	}

	bool Control::canDraw() const {
		return parent != nullptr && terminal != nullptr && 0 <= position.left && 0 <= position.top && !suppressDraw;
	}

	void Control::resize() {
		resize(position);
	}

	void Control::move(int left, int top) {
		position.left = left;
		position.top = top;
	}

	void Control::focus() {
		if (terminal)
			terminal->focus(this);
	}

	void Control::setParent(Container *parent_) {
		Child::setParent(parent_);
		if (parent_ != nullptr) {
			if (Terminal *parent_term = parent_->getTerminal())
				setTerminal(parent_term);
		}
	}

	void Control::jump() {
		position.jump();
	}

	void Control::jumpFocus() {
		jump();
	}

	void Control::clearRect() {
		if (!terminal)
			return;

		tryMargins([&, this]() {
			if (atLeft() && atRight()) {
				// Galaxy brain trickery here. If this control is as wide as the entire terminal, we can vscroll the
				// contents into oblivion very efficiently.
				terminal->vscroll(position.height);
			} else if (atLeft()) {
				// If we're at the left, we can clear each line from the end of the line to the left edge of the screen.
				terminal->jump(position.width - 1, 0);
				for (int i = 0; i < position.height; ++i) {
					if (i)
						terminal->down();
					terminal->clearLeft();
				}
			} else if (atRight()) {
				// If we're at the right, we can clear each line from the start of the line to the right edge.
				terminal->jump(position.left, 0);
				for (int i = 0; i < position.height; ++i) {
					if (i)
						terminal->down();
					terminal->clearRight();
				}
			} else {
				// If we're at neither edge, we have to print a total of width*height spaces. Very sad.
				std::string spaces(static_cast<size_t>(position.width), ' ');
				for (int i = 0; i < position.height; ++i) {
					terminal->jump(position.left, i);
					*terminal << spaces;
				}
			}
		});
	}

	void Control::flush() {
		if (terminal != nullptr)
			terminal->flush();
	}

	bool Control::hasFocus() const {
		return terminal && terminal->hasFocus(this);
	}

	bool Control::atRight() const {
		return terminal != nullptr && position.left + position.width == terminal->getCols();
	}

	bool Control::atLeft() const {
		return position.left == 0;
	}

	void Control::setMargins() {
		if (terminal != nullptr) {
			terminal->enableHmargins();
			terminal->margins(position.top, position.bottom(), position.left, position.right());
			terminal->setOrigin();
			inMargins = true;
		}
	}

	void Control::setHmargins() {
		if (terminal != nullptr) {
			terminal->enableHmargins();
			terminal->hmargins(position.left, position.right());
			terminal->setOrigin();
		}
	}

	void Control::resetMargins() {
		if (terminal != nullptr) {
			terminal->resetOrigin();
			terminal->margins();
			terminal->disableHmargins();
			inMargins = false;
		}
	}

	ssize_t Control::getIndex() const {
		if (parent != nullptr && !ignoreIndex) {
			ssize_t i = 0;
			for (auto iter = parent->begin(); iter != parent->end(); ++iter, ++i) {
				if (*iter == this)
					return i;

				if ((*iter)->ignoreIndex)
					--i;
			}
		}

		return -1;
	}

	void swap(Haunted::UI::Control &left, Haunted::UI::Control &right) {
		swap(static_cast<Haunted::UI::Child &>(left), static_cast<Haunted::UI::Child &>(right));
		std::swap(left.terminal,  right.terminal);
		std::swap(left.name,      right.name);
		std::swap(left.position,  right.position);
		std::swap(left.inMargins, right.inMargins);
	}
}
