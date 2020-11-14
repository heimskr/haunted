#ifndef TESTS_HAUNTED_H_
#define TESTS_HAUNTED_H_

#include <functional>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include <cassert>

#include "haunted/core/Terminal.h"
#include "haunted/core/Util.h"
#include "haunted/ui/Textbox.h"

#include "lib/formicine/ansi.h"

namespace Haunted::Tests {
	/**
	 * Contains general utilities for testing.
	 */
	class Testing {
		private:
			size_t totalPassed = 0, totalFailed = 0;

			// multi_apply is based on the possible implementation for std::apply at cppreference:
			// https://en.cppreference.com/w/cpp/utility/apply
			template <typename Fn, typename Trg, typename Tup>
			constexpr decltype(auto) multi_apply(Fn &&func, Trg *target, Tup &&tuple) {
				return multi_apply_impl(std::forward<Fn>(func), target, std::forward<Tup>(tuple),
					std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tup>>> {});
			}

			template <typename Fn, typename Trg, typename Tup, size_t... I>
			constexpr decltype(auto) multi_apply_impl(Fn &&func, Trg *target, Tup &&tuple, std::index_sequence<I...>) {
				return std::invoke(std::forward<Fn>(func), target, std::get<I>(std::forward<Tup>(tuple))...);
			}

		public:
			/** Stringifies a pair of integers. */
			static std::string stringify(const std::pair<int, int> &p) {
				return "{" + std::to_string(p.first) + ", " + std::to_string(p.second) + "}";
			}

			/** Stringifies a string by surrounding it with double quotes. */
			static std::string stringify(const std::string &str) {
				std::string escaped("");
				for (char c: str) {
					switch (c) {
						case '"':  escaped += "\\\""_bd; break;
						case '\n': escaped += "\\n"_bd;  break;
						case '\r': escaped += "\\r"_bd;  break;
						case '\t': escaped += "\\t"_bd;  break;
						case '\\': escaped += "\\\\"_bd;  break;
						default:   escaped += c;
					}
				}

				return "\""_d + escaped + "\""_d;
			}

			/** Stringifies a bool into a single letter (T or F). */
			template <typename T, std::enable_if_t<std::is_same<T, bool>::value, int> = 0>
			static std::string stringify(T b) {
				return b? "⊤"_b : "⊥"_b;
			}

			template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0,
			                      std::enable_if_t<!std::is_same<T, bool>::value, int> = 0>
			static std::string stringify(const T n) {
				return std::to_string(n);
			}

			template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
			static std::string stringify(const T n) {
				std::string stringified = std::to_string(n);
				while (stringified.back() == '0')
					stringified.pop_back();
				return stringified;
			}

			template <typename T, std::enable_if_t<std::is_class<T>::value, int> = 0,
			                      std::enable_if_t<!std::is_convertible<T, std::string>::value, int> = 0>
			static std::string stringify(const T &o) {
				return std::string(o);
			}

			template <typename E, std::enable_if_t<std::is_enum<E>::value, int> = 0>
			static std::string stringify(E e) {
				return stringify(static_cast<int>(e));
			}

			template <template <typename... T> typename C>
			static std::string stringify(Haunted::UI::TextLine<C> *tl) {
				return tl? std::to_string(tl->getContinuation()) + ":["_d + std::string(*tl) + "]"_d : "null";
			}

			template <typename T>
			static std::string stringify(std::shared_ptr<T> ptr) {
				return stringify(*ptr);
			}

			template <typename T>
			static std::string stringify(std::unique_ptr<T> ptr) {
				return stringify(*ptr);
			}

			template <typename A, typename B>
			static std::string stringify(const std::pair<A, B> &p) {
				return "{"_bd + stringify(p.first) + ", "_d + stringify(p.second) + "}"_bd;
			}

			template <typename T>
			static std::string stringify(const std::initializer_list<T> &list) {
				std::ostringstream oss;

				oss << "{"_bd;

				bool first = true;
				for (const T &item: list) {
					if (!first)
						oss << ", "_d;
					else
						first = false;

					oss << stringify(item);
				}

				oss << "}"_bd;
				return oss.str();
			}

			template <typename... Ts>
			static std::string stringify(const std::tuple<Ts...> &tuple) {
				return stringify_impl(tuple, std::index_sequence_for<Ts...> {});
			}

			template <typename... Ts, size_t... S>
			static std::string stringify_impl(const std::tuple<Ts...> &tuple, std::index_sequence<S...>) {
				std::ostringstream oss;
				oss << "{"_bd;
				(void)(std::initializer_list<int> {
					(oss << (S == 0? "" : ", "_d) << stringify(std::get<S>(tuple)), 0)...
				});
				oss << "}"_bd;
				return oss.str();
			}

			// We want to be able to compare the pairs returned by line_at_row as equal even if the addresses of the
			// pointers differ, as long as the content of the TextLines is the same.
			template <template <typename... T> typename C>
			using tl_pair = std::pair<Haunted::UI::TextLine<C> *, int>;
			template <template <typename... T> typename C>
			static bool equal(tl_pair<C> left, tl_pair<C> right) {
				return left.second == right.second && *left.first == *right.first;
			}

			template <typename L, typename R>
			static bool equal(const L &left, const R &right) {
				return left == right;
			}

			/** Whether to display results on destruction. */
			bool autodisplay;

			Testing(bool autodisplay = true): autodisplay(autodisplay) {}

			~Testing() {
				if (autodisplay && (totalFailed != 0 || totalPassed != 0)) {
					ansi::out << ansi::endl;
					display_results();
				}
			}

			/** Runs a set of tests and displays the results. */
			template <typename O, typename... I>
			bool check(const std::vector<std::pair<std::tuple<I...>, O>> &pairs,
			           std::function<O(const std::tuple<I...> &)> fn, const std::string &fn_name) {
				using namespace ansi;

				if (pairs.size() == 0) {
					ansi::out << ansi::warn << "No tests given." << ansi::endl;
					return false;
				}

				const std::string prefix = fn_name.empty()? "fn" : fn_name;
				size_t passed = 0, failed = 0;

				size_t max_length = 0;
				for (const std::pair<std::tuple<I...>, O> &p: pairs) {
					const size_t length = ansi::length(stringify(p.first));
					if (length > max_length)
						max_length = length;
				}

				const std::string padding(max_length, ' ');

				for (const std::pair<std::tuple<I...>, O> &p: pairs) {
					const std::tuple<I...> &input = p.first;
					const O &expected = p.second;
					const size_t length = ansi::length(stringify(input));
					try {
						const O &actual = fn(input);
						if (equal(expected, actual)) {
							display_passed(stringify(input), stringify(actual), prefix,
							               padding.substr(0, max_length - length));
							passed++;
						} else {
							display_failed(stringify(input), stringify(actual), stringify(expected), prefix,
							               padding.substr(0, max_length - length));
							failed++;
						}
					} catch (std::exception &err) {
						display_failed(stringify(input), "\e[31;1m" + Util::demangleObject(err) + "\e[22m: " +
						               std::string(err.what()) + "\e[0m", stringify(expected), prefix,
						               padding.substr(0, max_length - length));
						failed++;
					}
				}

				totalPassed += passed;
				totalFailed += failed;
				return failed == 0;
			}

			/** Runs a set of tests and displays the results. */
			template <typename O, typename... I>
			bool check(const std::vector<std::pair<std::tuple<I...>, O>> &pairs, std::function<O(I...)> fn,
			const std::string &fn_name) {
				return check(pairs, [&](const std::tuple<I...> &input) -> O {
					return std::apply(fn, input);
				}, fn_name);
			}

			/** Runs a set of tests and displays the results. */
			template <typename O, typename... I>
			bool check(const std::vector<std::pair<std::tuple<I...>, O>> &pairs, O(*fn)(I...),
			const std::string &fn_name) {
				return check(pairs, std::function<O(const std::tuple<I...> &)>([&](const std::tuple<I...> &input) -> O {
					return std::apply(std::function<O(I...)>(fn), input);
				}), fn_name);
			}

			/** Runs a set of tests and displays the results. */
			template <typename T, typename O, typename... I>
			bool check(const std::vector<std::pair<std::tuple<I...>, O>> &pairs, O(T::*fn)(I...), T *target,
			const std::string &fn_name) {
				return check(pairs, std::function<O(const std::tuple<I...> &)>([&](const std::tuple<I...> &args) -> O {
					return multi_apply(fn, target, args);
				}), fn_name);
			}
			/** Used for testing a single expected value with an actual value. */
			template <typename A, typename E>
			bool check(const A &actual, const E &expected, const std::string &fn_name) {
				bool result = equal(actual, expected);
				if (result) {
					++totalPassed;
					ansi::out << ansi::good << fn_name << " " << "== "_d << ansi::green(stringify(actual))
					          << ansi::endl;
				} else {
					++totalFailed;
					ansi::out << ansi::bad << fn_name << " " << "== "_d << ansi::red(stringify(actual))
					          << " (expected " << ansi::bold(stringify(expected)) << ")" << ansi::endl;
				}

				return result;
			}

			/** Used for testing a single expected value with an actual value. */
			template <typename T>
			bool check(const T &actual, const T &expected, const std::string &fn_name) {
				return check<T, T>(actual, expected, fn_name);
			}

			/** Used to check whether a function throws an exception of a given type. */
			template <typename O, typename... I>
			bool check(const std::string &fn_name, const std::type_info &errtype, const std::string &what,
			           std::function<O(I...)> fn, I... args) {
				const std::string demangled = Util::demangle(std::string(errtype.name()));
				try {
					const std::string returned = stringify(fn(args...));
					ansi::out << ansi::bad << fn_name << " == "_d << ansi::red(returned) << " (expected " << demangled;
					if (!what.empty())
						ansi::out << ", \"" << what << "\"";
					ansi::out << ")";
				} catch (std::exception &exc) {
					std::string message = exc.what();
					if (typeid(exc) == errtype && (what.empty() || what == std::string(exc.what()))) {
						ansi::out << ansi::good << fn_name << " throws "_d << ansi::green(demangled);
						if (!message.empty())
							ansi::out << " (" << (what.empty()? message : ansi::green(message)) << ")";
						ansi::out << ansi::endl;
						++totalPassed;
						return true;
					}

					ansi::out << ansi::bad << fn_name << " throws "_d
					          << ansi::wrap(demangled, typeid(exc) == errtype? ansi::color::green : ansi::color::red);
					if (!message.empty()) {
						ansi::out << " (" << ansi::wrap(message, message == what? ansi::color::green : ansi::color::red)
						          << ")";
					}
				}

				ansi::out << ansi::endl;
				++totalFailed;
				return false;
			}

			/** Used to check whether a function throws an exception of a given type. */
			template <typename T, typename O, typename... I>
			bool check(const std::string &fn_name, const std::type_info &errtype, const std::string &what,
			           T *target, O(T::*fn)(I...), I... args) {
				return check(fn_name, errtype, what, std::function<O(I...)>([&](I... input) -> O {
					return (target->*fn)(input...);
				}), args...);
			}

			/** Used to check whether a function throws an exception of a given type. */
			template <typename T, typename O, typename... I>
			bool check(const std::string &fn_name, const std::type_info &errtype, const std::string &what,
			           const T *target, O(T::*fn)(I...) const, I... args) {
				return check(fn_name, errtype, what, std::function<O(I...)>([&](I... input) -> O {
					return (target->*fn)(input...);
				}), args...);
			}

			/** Used to check whether a function throws an exception of a given type. */
			template <typename O, typename... I>
			bool check(const std::string &fn_name, const std::type_info &errtype, const std::string &what,
			           O(*fn)(I...), I... args) {
				return check(fn_name, errtype, what, std::function<O(I...)>(fn), args...);
			}

			/** Used to check whether a function throws an exception of a given type. */
			template <typename O, typename... I>
			bool check(const std::string &fn_name, const std::type_info &errtype, const std::string &what,
			           O(*fn)(const I &...), const I &... args) {
				return check(fn_name, errtype, what, std::function<O(I...)>(fn), args...);
			}

			void display_results() const {
				if (totalFailed == 0 && totalPassed == 0) {
					ansi::out << ansi::warn << "No tests were run.\n";
				} else if (totalFailed == 0) {
					if (totalPassed == 1)
						ansi::out << ansi::good << "Test passed.\n";
					else
						ansi::out << ansi::good << "All " << totalPassed << " tests passed.\n";
				} else if (totalPassed == 0) {
					if (totalFailed == 1)
						ansi::out << ansi::bad << "Test failed.\n";
					else
						ansi::out << ansi::bad << "All " << totalFailed << " tests failed.\n";
				} else {
					ansi::out << ansi::warn
						<< "Passed " << wrap(std::to_string(totalPassed), ansi::color::green)
						<< ", failed " << wrap(std::to_string(totalFailed), ansi::color::red)
						<< " (" << ansi::style::bold << std::setprecision(4)
						<< (totalPassed * 100.0 / (totalPassed + totalFailed)) << "%" >> ansi::style::bold
						<< ")" << std::defaultfloat << ansi::endl;
				}
			}

			static void display_failed(const std::string &input, const std::string &actual, const std::string &expected,
			                           const std::string &prefix, const std::string &padding,
			                           const std::exception *err = nullptr) {
				ansi::out << ansi::bad << prefix << ansi::parens << ansi::bold(input) << padding << " == "_d;

				if (err != nullptr)
					ansi::out << ansi::red(ansi::bold(Util::demangleObject(err)) + ": " + std::string(err->what()));
				else
					ansi::out << ansi::red(actual);

				ansi::out << " Expected: "_d << ansi::yellow(expected) << ansi::endl;
			}

			static void display_passed(const std::string &input, const std::string &actual, const std::string &prefix,
			                           const std::string &padding) {
				ansi::out << ansi::good << prefix << ansi::parens << ansi::bold(input) << padding << " == "_d
				          << ansi::green(actual) << ansi::endl;
			}
	};

	class maintest {
		public:
			static std::pair<int, int> parse_csi(const std::string &);
			static void test_textinput(Terminal &);
			static void test_key(Terminal &);
			static void test_cursor(Terminal &);
			static void test_margins(Terminal &);
			static void test_textbox(Terminal &);
			static void test_expandobox(Terminal &);
			static void unittest_csiu(Testing &);
			static void unittest_textbox(Testing &);
			static void unittest_expandobox(Testing &);
			static void unittest_ustring(Testing &);
	};
}

#endif
