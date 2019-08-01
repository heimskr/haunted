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
#include "core/util.h"

namespace haunted::tests {
	/**
	 * Contains general utilities for testing.
	 */
	class testing {
		private:
			size_t total_passed = 0, total_failed = 0;

		public:
			static std::string stringify(const std::pair<int, int> &p);
			static std::string stringify(const std::string &);
			static std::string stringify(bool);

			template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
			static std::string stringify(const T n) {
				return std::to_string(n);
			}

			template <typename T,
				std::enable_if_t<std::is_class<T>::value, int> = 0,
				std::enable_if_t<!std::is_convertible<T, std::string>::value, int> = 0>
			static std::string stringify(const T &o) {
				return std::string(o);
			}

			template <typename A, typename B>
			static std::string stringify(const std::pair<A, B> &p) {
				using namespace ansi;
				return wrap("{", dim) + stringify(p.first) + wrap(", ", dim) + stringify(p.second) + wrap("}", dim);
			}

			/** Whether to display results on destruction. */
			bool autodisplay;

			testing(bool autodisplay): autodisplay(autodisplay) {}
			testing(): testing(true) {}

			~testing();

			/**
			 * Runs a set of tests and displays the results.
			 * @param  pairs    A vector of {input data, expected results} pairs.
			 * @param  fn       The function whose output will be compared to the expected results.
			 * @param  fn_name  The name of the function.
			 * @return True if no tests failed.
			 */
			template <typename I, typename O>
			bool check(const std::vector<std::pair<I, O>> &pairs, std::function<O(I)> fn,
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
					size_t length = stringify(input).size();
					try {
						const O &actual = fn(input);
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
						display_failed(stringify(input), "\e[31;1m" + util::demangle_object(err) + "\e[22m: " +
						               std::string(err.what()) + "\e[0m", stringify(expected), prefix,
						               padding.substr(0, max_length - length));
						failed++;
					}
				}

				total_passed += passed;
				total_failed += failed;
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
			bool check(const std::vector<std::pair<I, O>> &pairs, O(T::*fn)(I), T *target, const std::string &fn_name) {
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
						display_failed(stringify(input), "\e[31;1m" + util::demangle_object(err) + "\e[22m: " +
						               std::string(err.what()) + "\e[0m", stringify(expected), prefix,
						               padding.substr(0, max_length - length));
						failed++;
					}
				}

				total_passed += passed;
				total_failed += failed;
				return failed == 0;
			}

			/** Used for testing a single expected value with an actual value. */
			template <typename T>
			bool check(const T &actual, const T &expected, const std::string &fn_name) {
				using namespace ansi;
				bool result = actual == expected;
				if (result) {
					++total_passed;
					out << good << fn_name << " " << wrap("== ", dim) << wrap(stringify(actual), green) << endl;
				} else {
					++total_failed;
					out << bad << fn_name << " " << wrap("== ", dim) << wrap(stringify(actual), red)
					    << " (expected " << wrap(stringify(expected), bold) << ")" << endl;
				}

				return result;
			}

			/** Used to check whether a function throws an exception of a given type. */
			template <typename T, typename R, typename... A>
			bool check(const std::string &fn_name, const std::type_info &errtype, R(T::*fn)(A...), T *target, const std::string &what, A... args) {
				using namespace ansi;
				const std::string demangled = util::demangle(std::string(errtype.name()));

				try {
					const std::string returned = stringify((target->*fn)(args...));
					out << bad << fn_name << wrap(" == ", dim) << wrap(returned, red) << " (expected "
						<< demangled;
					if (!what.empty())
						out << ", \"" << what << "\"";
					out << ")";
				} catch (std::exception &exc) {
					std::string message = exc.what();
					if (typeid(exc) == errtype && (what.empty() || what == std::string(exc.what()))) {
						out << good << fn_name << wrap(" throws ", dim) << wrap(demangled, green);
						if (!message.empty())
							out << " (" << (what.empty()? message : wrap(message, green)) << ")";
						out << endl;
						++total_passed;
						return true;
					}

					out << bad << fn_name << wrap(" throws ", dim)
					    << wrap(demangled, typeid(exc) == errtype? green : red);
					if (!message.empty())
						out << " (" << wrap(message, message == what? green : red) << ")";
				}

				out << endl;
				++total_failed;
				return false;
			}

			void display_results() const;
			static void display_failed(const std::string &input, const std::string &actual, const std::string &expected,
			                           const std::string &prefix, const std::string &padding,
			                           const std::exception *err = nullptr);
			static void display_passed(const std::string &input, const std::string &actual, const std::string &prefix,
			                           const std::string &padding);
	};

	class maintest {
		public:
			static std::pair<int, int> parse_csi(const std::string &);
			static void test_csiu();
			static void test_textinput(terminal &);
			static void test_key(terminal &);
			static void test_cursor(terminal &);
			static void test_margins(terminal &);
			static void test_textbox(terminal &);
			static void unittest_textbox(terminal &);
			static void test_expandobox(terminal &);
	};
}

#endif
