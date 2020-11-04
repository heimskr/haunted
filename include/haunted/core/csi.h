#ifndef HAUNTED_CORE_CSI_H_
#define HAUNTED_CORE_CSI_H_

#include <string>

#include "haunted/core/Key.h"

// Enables a workaround for #7885, which was fixed in 3.3.20190615-nightly.
// #define ITERM_HACK

namespace Haunted {
	enum class CSIType {U, Special, ReallySpecial};

	class CSI {
		private:
			void parseU(const std::string &);
			void parseSpecial(const std::string &);

			static void scanNumber(unsigned int &, ssize_t &, const std::string &);

		public:
			unsigned int first, second;
			char suffix;
			CSIType type;

			CSI(int first, int second, char suffix);

			/** Parses a CSI sequence. Throws an exception if the input is invalid. */
			CSI(const std::string &);

			Key getKey() const;

			operator std::pair<int, int>() const;


			/** Determines whether a string looks like a CSI u sequence (numbers, semicolon, numbers, letter). */
			static bool isCSIu(const std::string &);
	};
}

#endif
