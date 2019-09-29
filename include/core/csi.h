#ifndef HAUNTED_CORE_CSI_H_
#define HAUNTED_CORE_CSI_H_

#include <string>

#include "core/key.h"

// Enables a workaround for #7885, which was fixed in 3.3.20190615-nightly.
// #define ITERM_HACK

namespace haunted {
	enum class csi_type {u, special, really_special};

	class csi {
		private:
			void parse_u(const std::string &);
			void parse_special(const std::string &);

			static void scan_number(unsigned int &, ssize_t &, const std::string &);

		public:
			unsigned int first, second;
			char suffix;
			csi_type type;

			csi(int first, int second, char suffix);

			/** Parses a CSI sequence. Throws an exception if the input is invalid. */
			csi(const std::string &);

			key get_key() const;

			operator std::pair<int, int>() const;


			/** Determines whether a string looks like a CSI u sequence (numbers, semicolon, numbers, letter). */
			static bool is_csiu(const std::string &);
	};
}

#endif
