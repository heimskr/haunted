#ifndef TESTS_HAUNTED_H_
#define TESTS_HAUNTED_H_

#include <functional>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include <cassert>

#include "formicine/ansi.h"
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

			template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
			static std::string stringify(const T n) {
				return std::to_string(n);
			}

			template <typename T, std::enable_if_t<std::is_class<T>::value, int> = 0>
			static std::string stringify(const T &o) {
				return std::string(o);
			}

			template <typename A, typename B>
			static std::string stringify(const std::pair<A, B> &p) {
				return "{" + stringify(p.first) + ", " + stringify(p.second) + "}";
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
			static bool check(const std::vector<std::pair<I, O>> &pairs, std::function<O(I)> fn,
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
					I &input = p.first;
					O &expected = p.second;
					O &actual = fn(input);
					size_t length = stringify(input).size();
					if (expected == actual) {
						display_passed(input, actual, prefix, padding.substr(0, max_length - length));
						passed++;
					} else {
						display_failed(input, actual, expected, prefix, padding.substr(0, max_length - length));
						failed++;
					}
				}

				out << "\n";
				display_results(passed, failed);

				return failed == 0;
			}

			/**
			 * Runs a set of tests and displays the results.
			 * @param  pairs    A vector of {input data, expected results} pairs.
			 * @param  fn       The function whose output will be compared to the expected results.
			 * @param  fn_name  The name of the function.
			 * @return True if no tests failed.
			 */
			template <typename I, typename O, typename T>
			static bool check(const std::vector<std::pair<I, O>> &pairs, O(T::*fn)(I), T *target, const std::string &fn_name) {
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
					size_t length = stringify(input).size();
					try {
						const O &actual = std::invoke(fn, *target, input);
						if (expected == actual) {
							display_passed(stringify(input), stringify(actual), prefix,
							               padding.substr(0, max_length - length));
							passed++;
						} else {
							display_failed(stringify(input), stringify(actual), stringify(expected), prefix,
							               padding.substr(0, max_length - length));
							failed++;
						}
					} catch (std::exception &err) {
						display_failed(stringify(input), "\e[31mException:\e[0m " + std::string(err.what()),
						               stringify(expected), prefix, padding.substr(0, max_length - length));
						failed++;
					}
				}

				out << "\n";
				display_results(passed, failed);

				return failed == 0;
			}

			static void display_results(size_t passed, size_t failed);
			static void display_failed(const std::string &input, const std::string &actual, const std::string &expected,
			                           const std::string &prefix, const std::string &padding);
			static void display_passed(const std::string &input, const std::string &actual, const std::string &prefix,
			                           const std::string &padding);
	};

	class maintest {
		public:
			static void test_csiu();
			static void test_textinput(terminal &);
			static void test_key(terminal &);
			static void test_cursor(terminal &);
			static void test_margins(terminal &);
			static void test_textbox(terminal &);
			static void unittest_textbox(terminal &);
	};
}

#endif
