// Copyright © 2020 Miguel González Cuadrado <mgcuadrado@gmail.com>

// This file is part of Testudo.

//     Testudo is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.

//     Testudo is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.

//     You should have received a copy of the GNU General Public License
//     along with Testudo.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MGCUADRADO_TESTUDO_BASE_HEADER_
#define MGCUADRADO_TESTUDO_BASE_HEADER_

// This module gives basic definitions for "is_valid()", "are_equal()",
// "absdiff()", and "to_text()".  The "is_valid()" function determines whether
// an argument to the "check()" macro is valid or not: the "check()" macro will
// always report a check involving an invalid value as false, even if the
// comparison yielded true.  The "are_equal()" is equivalent by default to
// "operator==()".  The "absdiff()" function computes the scalar difference
// between two values, and is used to check whether a value is close to an
// expected value.  The "to_text()" function converts a value to a text, and is
// used by the test commands "show_value()", "check_approx()", and
// "check_approx_tol()".
//
// These functions can be overloaded for new types that are to be used in
// tests.  The overloads must be done in the same namespace as the new types,
// so they're found by argument-dependent lookup.  If the new types contain
// values of types already supported by overloads, the definitions for the new
// types should use the existing overloads.
//
// The default definition for "to_text()" uses redirection to output stream if
// available.  Therefore, if you define it for your new type, you don't have to
// overload "to_text()" for it.  If "to_text()" isn't overloaded, and if
// redirection to output stream isn't defined for a type, it will return the
// placeholder "<...>".

#include "testudo_macros.h"
#include <string>
#include <sstream>

namespace testudo___implementation {

  // values are valid by default:
  template <typename T>
  bool is_valid_testudo(T const &)
    { return true; }

  template <typename T>
  bool is_valid(T const &t)
    { return is_valid_testudo(t); }

  template <typename T, typename U>
  bool are_equal_testudo(T const &t, U const &u)
    { return t==u; }

  template <typename T, typename U>
  bool are_equal(T const &t, U const &u)
    { return are_equal_testudo(t, u); }

  // scalar differences between two values "a" and "b" for testing evaluation
  // are computed using "absdiff(a, b)"; users must overload this function for
  // their own types; it must always return "double", and if possible, use
  // other overloads of "absdiff()" for simpler types in its definition
  inline double absdiff_testudo(double a, double b) {
    // equivalent to "std::abs(a-b)", but avoids having to "#include <cmath>":
    return a<b ? b-a : a-b;
  }

  template <typename T, typename U>
  double absdiff(T const &t, U const &u)
    { return absdiff_testudo(t, u); }

  namespace implementation {
    template <typename...>
    using void_t=void;

    // detect if a type supports "ostream <<"
    template <typename T, typename=void>
    struct has_textual_representation_t : std::false_type { };
    template <typename T>
    struct has_textual_representation_t<
      T, void_t<decltype(std::declval<std::ostream &>() << std::declval<T>())>>
      : std::true_type { };
    template <typename T>
    constexpr bool has_textual_representation_v=
      has_textual_representation_t<T>::value;

    // detect if a type supports ".what()" (meant for exceptions)
    template <typename T, typename=void>
    struct has_what_representation_t : std::false_type { };
    template <typename T>
    struct has_what_representation_t<
      T, void_t<decltype(std::declval<T>().what())>>
      : std::true_type { };
    template <typename T>
    constexpr bool has_what_representation_v=
      has_what_representation_t<T>::value;
  }

  // textual representation for a value; if available, use "ostream <<";
  // otherwise, return a placeholder
  template <typename T>
  std::string to_text_testudo(T const &t) {
    if constexpr (implementation::has_textual_representation_v<T>) {
      std::ostringstream oss;
      oss << std::boolalpha << t;
      return oss.str();
    }
    else
      return "<...>";
  }

  template <typename T>
  std::string to_text(T const &t)
    { return to_text_testudo(t); }

  // textual representation for an exception "e"; if available, use
  // "to_text(e.what())"; otherwise, "to_text(e)"
  template <typename E>
  std::string exception_to_text(E const &e) {
    if constexpr (implementation::has_what_representation_v<E>) {
      return to_text(e.what());
    }
    else
      return to_text(e);
  }

}

testudo___BRING(is_valid_testudo)
testudo___BRING(to_text)
testudo___BRING(absdiff)

#endif
