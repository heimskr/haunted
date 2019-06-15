#ifndef HAUNTED_CORE_CSI_H_
#define HAUNTED_CORE_CSI_H_

#include <string>

#include "core/key.h"

namespace haunted {
	enum class csi_type {u, special, really_special};

	class csi {
		public:
			int first, second;
			char suffix;
			csi_type type;

			csi(int first, int second, char suffix);

			ktype get_key() const;

			/** Parses a CSI sequence. Throws an exception if the input is invalid. */
			static csi parse(const std::string &);

			/** Determines whether a string looks like a CSI u sequence (numbers, semicolon, numbers, letter). */
			static bool is_csiu(const std::string &);
	};
}

#endif
