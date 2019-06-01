#ifndef TESTS_HAUNTED_H_
#define TESTS_HAUNTED_H_

#include <functional>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include <cassert>

#include "lib/ansi.h"
#include "core/terminal.h"

namespace haunted::tests {
	/**
	 * Contains general utilities for testing.
	 */
	class testing {
		private:
			static std::string stringify(const std::pair<int, int> &p);
			static std::string stringify(const std::string &);
			static std::string stringify(bool);

			template <typename T>
			static std::string stringify(const T &o) {
				return std::string(o);
			}

		public:
			/**
			 * Runs a set of tests and displays the results.
			 * @param  pairs    A vector of {input data, expected results} pairs.
			 * @param  fn       The function whose output will be compared to the expected results.
			 * @param  fn_name  The name of the function.
			 * @return True if no tests failed.
			 */
			template <typename I, typename O>
			static bool check(const std::vector<std::pair<I, O>> &pairs, std::function<O(const I &)> fn,
			                  const std::string &fn_name) {
				using namespace ansi;

				if (pairs.size() == 0) {
					out << warn << "No tests given." << endl;
					return false;
				}

				const std::string prefix = fn_name.empty()? "fn" : fn_name;
				size_t passed = 0, failed = 0;

				size_t max_length = 0;
				for (const std::pair<I, O> &p: pairs) {
					size_t length = stringify(p.first).size();
					if (length > max_length)
						max_length = length;
				}

				const std::string padding(max_length, ' ');

				for (const std::pair<I, O> &p: pairs) {
					const I &input = p.first;
					const O &expected = p.second;
					const O &actual = fn(input);
					size_t length = stringify(input).size();
					if (expected == actual) {
						out << good << prefix << parens << wrap(stringify(input), bold)
						    << padding.substr(0, max_length - length)
						    << wrap(" == ", dim) << wrap(stringify(actual), green) << endl;
						passed++;
					} else {
						out << bad << prefix << parens << wrap(stringify(input), bold)
						    << padding.substr(0, max_length - length)
						    << wrap(" == ", dim) << wrap(stringify(actual), red)
						    << " (expected " << wrap(stringify(expected), bold) << ")" << endl;
						failed++;
					}
				}

				out << "\n";

				if (failed == 0 && passed == 0) {
					out << warn << "No tests were run.\n";
				} else if (failed == 0) {
					if (passed == 1)
						out << good << "Test passed.\n";
					else
						out << good << "All " << passed << " tests passed.\n";
				} else if (passed == 0) {
					if (failed == 1)
						out << bad << "Test failed.\n";
					else
						out << bad << "All " << failed << " tests failed.\n";
				} else {
					out << warn
					    << "Passed " << wrap(std::to_string(passed), green)
					    << ", failed " << wrap(std::to_string(failed), red)
					    << " (" << bold << std::setprecision(4) << (passed * 100.0 / (passed + failed)) << "%" >> bold
					    << ")" << std::defaultfloat << endl;
				}

				return failed == 0;
			}
	};

	class maintest {
		public:
			static void test_csiu();
			static void test_textinput();
	};
}

#endif
