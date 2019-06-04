#ifndef HAUNTED_UI_TEXTINPUT_H_
#define HAUNTED_UI_TEXTINPUT_H_

#include <cstddef>
#include <functional>
#include <ostream>
#include <string>
#include <unordered_set>

#include "../core/defs.h"
#include "../core/key.h"
#include "../lib/utf8.h"
#include "ui/container.h"
#include "ui/control.h"

namespace haunted::ui {
	class textinput: public virtual control {
		using update_fn = std::function<void(const utf8str &, int)>;

		private:
			static std::unordered_set<unsigned char> whitelist;
			
			std::string prefix;
			utf8str buffer;
			size_t cursor = 0, bytes_expected = 0;
			update_fn on_update;

			// Every time the textinput is redrawn, it
			// records the screen position of the cursor.
			point cursor_pos;

			void update();

		public:
			std::string unicode_buffer;

			textinput(container *parent, const std::string &buffer, size_t cursor);
			textinput(container *parent, const std::string &buffer):
				textinput(parent, buffer, 0) {}
			textinput(container *parent): textinput(parent, "") {}

			textinput(position pos, const std::string &buffer, size_t cursor):
				control(pos), buffer(buffer), cursor(cursor) {}
			textinput(position pos, const std::string &buffer):
				textinput(pos, buffer, 0) {}
			textinput(position pos, size_t cursor): textinput(pos, "", cursor) {}
			textinput(position pos): textinput(pos, "", 0) {}

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

			bool on_key(key &) override;
			void draw() override;

			std::string dbg_render(bool = true) const;

			friend std::ostream & operator<<(std::ostream &os,
				const textinput &input);
	};
}

#endif
