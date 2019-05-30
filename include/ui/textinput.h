#ifndef UI_TEXTINPUT_H_
#define UI_TEXTINPUT_H_

#include <cstddef>
#include <functional>
#include <ostream>
#include <string>
#include <unordered_set>

#include "ui/defs.h"
#include "lib/utf8.h"

namespace spjalla {
	class textinput {
		using update_fn = std::function<void(const utf8str &, int)>;

		private:
			std::string prefix;
			utf8str buffer;
			size_t cursor = 0, bytes_expected = 0;
			update_fn on_update;
			void update();
			static std::unordered_set<unsigned char> whitelist;

		public:
			std::string unicode_buffer;
			textinput(std::string buffer_, size_t cursor_): buffer(buffer_), cursor(cursor_) {}
			textinput(std::string buffer_): textinput(buffer_, 0) {}
			textinput(size_t cursor_): textinput("", cursor_) {}
			textinput(): textinput("", 0) {}

			operator std::string() const;

			void listen(const update_fn &);
			void move_to(size_t);
			void insert(const std::string &);
			void insert(unsigned char);
			void clear();
			void erase_word();
			void erase();
			std::string get_text() const;
			void set_text(const std::string &);
			void left();
			void right();
			void start();
			void end();
			void prev_word();
			void next_word();
			size_t length() const;
			size_t size() const;
			char prev_char() const;
			char next_char() const;
			size_t get_cursor() const;

			std::string dbg_render(bool = true) const;

			friend std::ostream & operator<<(std::ostream &os, const textinput &input);
	};
}

#endif
