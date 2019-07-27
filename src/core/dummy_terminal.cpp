#include "core/dummy_terminal.h"

namespace haunted {
	void dummy_terminal::apply() {}
	void dummy_terminal::reset() {}
	void dummy_terminal::winch(int, int) {}
	void dummy_terminal::setattr(const termios &) {}
	void dummy_terminal::cbreak() {}
	void dummy_terminal::watch_size() {}
	void dummy_terminal::redraw() {}
	void dummy_terminal::set_root(ui::control *) {}
	void dummy_terminal::draw() {}
	void dummy_terminal::start_input() {}
	void dummy_terminal::flush() {}
	void dummy_terminal::hmargins(size_t, size_t) {}
	void dummy_terminal::hmargins() {}
	void dummy_terminal::vmargins(size_t, size_t) {}
	void dummy_terminal::vmargins() {}
	void dummy_terminal::margins(size_t, size_t, size_t, size_t) {}
	void dummy_terminal::margins() {}
	void dummy_terminal::enable_hmargins() {}
	void dummy_terminal::disable_hmargins() {}
	void dummy_terminal::set_origin() {}
	void dummy_terminal::reset_origin() {}
	dummy_terminal::operator bool() const { return true; }
	terminal & dummy_terminal::operator>>(int &)  { return *this; }
	terminal & dummy_terminal::operator>>(char &) { return *this; }
	terminal & dummy_terminal::operator>>(key &)  { return *this; }
} 
