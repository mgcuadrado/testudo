// Copyright © 2020-2023 Miguel González Cuadrado <mgcuadrado@gmail.com>

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
// expected value.  The "to_text()" function converts a value to a text.
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
#include <tuple>
#include <cfloat>
#include <cassert>

namespace testudo___implementation {

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

  // check whether two values are closer than a tolerance
  template <typename T1, typename T2, typename TE>
  bool are_approx(T1 const &arg1, T2 const &arg2, TE max_error)
    { return (absdiff(arg1, arg2)<=max_error); }

  namespace implementation {
    template <typename...>
    using void_t=void;

#define define_support_for(name, ...)                                   \
    template <typename T1=void, typename T2=void, typename T3=void,     \
              typename=void>                                            \
    struct support_for_##name##_t : std::false_type { };                \
    template <typename T1, typename T2, typename T3>                    \
    struct support_for_##name##_t<T1, T2, T3,                           \
                                  void_t<decltype((__VA_ARGS__))>>      \
      : std::true_type { };                                             \
    template <typename T1=void, typename T2=void, typename T3=void>     \
    constexpr bool support_for_##name##_v=                              \
      support_for_##name##_t<T1, T2, T3>::value;                        \
    testudo___DECL_EAT_A_SEMICOLON

    define_support_for(
      textual_representation,
      std::declval<std::ostream &>() << std::declval<T1>());
    define_support_for(
      what_representation,
      std::declval<T1>().what());
    define_support_for(
      cbegin_cend,
      (++std::declval<T1>().cbegin())==std::declval<T1>().cend());
    define_support_for(
      tuple_size,
      std::tuple_size<T1>::value);

#undef define_support_for
  }

  // textual representation for a value; if available, use "ostream <<";
  // otherwise, return a placeholder
  template <typename T>
  std::enable_if_t<
    (not implementation::support_for_cbegin_cend_v<T>
     and not implementation::support_for_tuple_size_v<T>),
    void>
  to_stream_testudo(std::ostream &os, T const &t) {
    if constexpr (implementation::support_for_textual_representation_v<T>) {
      os << t;
    }
    else
      os << "<...>";
  }

  template <typename... T>
  void to_stream_testudo(std::ostream &os, std::basic_string<T...> const &s)
    { os << "\"" << s << "\""; }

  // a string type that will be printed by Testudo without surrounding quotes
  struct unquoted_text {
    std::string text;
  };
  inline void to_stream_testudo(std::ostream &os, unquoted_text const &ut)
    { os << ut.text; }

  template <typename T>
  inline unquoted_text unquoted(T const &s) { return {s}; }

  template <typename T>
  void to_stream(std::ostream &os, T const &t)
    { to_stream_testudo(os, t); }

  template <typename T>
  std::string to_text(std::ostream &fmt_os, T const &t) {
    std::ostringstream oss;
    oss.copyfmt(fmt_os);
    to_stream(oss, t);
    return oss.str();
  }

  extern std::ostringstream default_fmt_os;

  template <typename T>
  std::string to_text(T const &t) { return to_text(default_fmt_os, t); }


  // values are valid by default:
  template <typename T>
  std::enable_if_t<
    (not implementation::support_for_cbegin_cend_v<T>
     and not implementation::support_for_tuple_size_v<T>),
    bool>
  is_valid_testudo(T const &)
    { return true; }

  template <typename C, typename T, typename A>
  bool is_valid_testudo(std::basic_string<C, T, A> const &)
    { return true; }

  template <typename T>
  bool is_valid(T const &t)
    { return is_valid_testudo(t); }

}

/// support for STL tuple, pair, and containers

namespace testudo___implementation {

  template <typename A, std::size_t... i>
  bool is_valid_get(A const &a, std::index_sequence<i...>)
    { return (is_valid(std::get<i>(a)) and ...); }

  template <typename A>
  bool is_valid_sequence(A const &a) {
    for (auto const &e: a)
      if (not is_valid(e))
        return false;
    return true;
  }

  template <typename A, typename B, std::size_t... i>
  double absdiff_get(A const &a, B const &b, std::index_sequence<i...>)
    { return (absdiff(std::get<i>(a), std::get<i>(b))+...); }

  template <typename A, typename B>
  double absdiff_sequence(A const &a, B const &b) {
    double result=0.;
    auto ai=a.cbegin();
    auto bi=b.cbegin();
    for (; (ai not_eq a.cend()) and (bi not_eq b.cend()); ++ai, ++bi)
      result+=absdiff(*ai, *bi);
    if ((ai==a.cend()) and (bi==b.cend()))
      return result;
    else
      return DBL_MAX;
  }

  inline void to_stream_comma_separated(std::ostream &)
    { }
  template <typename T>
  void to_stream_comma_separated(std::ostream &os, T const &t)
    { to_stream(os, t); }
  template <typename F, typename... R>
  void to_stream_comma_separated(std::ostream &os, F const &f, R const &...r) {
    to_stream(os, f);
    os << ", ";
    to_stream_comma_separated(os, r...);
  }

  template <typename... T>
  std::string to_text_comma_separated(std::ostream &fmt_os, T const &...t) {
    std::ostringstream oss;
    oss.copyfmt(fmt_os);
    to_stream_comma_separated(oss, t...);
    return oss.str();
  }

  template <typename... T>
  std::string to_text_brackets(std::ostream &fmt_os, T const &...t)
    { return "("+to_text_comma_separated(fmt_os, t...)+")"; }

  template <typename Tu, std::size_t... i>
  void to_stream_get(std::ostream &os, Tu const &t, std::index_sequence<i...>)
    { to_stream_comma_separated(os, std::get<i>(t)...); }
  template <typename Tu,
            auto size=std::tuple_size<Tu>::value> // also valid for pairs
  void to_stream_get(std::ostream &os,
                     Tu const &t)
    { to_stream_get(os, t, std::make_index_sequence<size>()); }

  template <typename A>
  void to_stream_sequence(std::ostream &os,
                          A const &a) {
    if (not a.empty()) {
      bool first=true;
      for (auto const &e: a) {
        if (not first)
          os << ", ";
        to_stream(os, e);
        first=false;
      }
    }
  }

  template <typename Tu>
  std::string to_text_get(std::ostream &fmt_os, Tu const &t) {
    std::ostringstream oss;
    oss.copyfmt(fmt_os);
    to_stream_get(oss, t);
    return oss.str();
  }

}

namespace std {
  // overloads are in the "std" namespace, where the types they apply to are
  // defined, so they're found by argument-dependent lookup

  /// "is_valid()" overloads

  // tuples and pairs
  template <typename Tu,
            auto size=std::tuple_size<Tu>::value>
  std::enable_if_t<
    testudo___implementation::implementation
      ::support_for_tuple_size_v<Tu>,
    bool>
  is_valid_testudo(Tu const &tu) {
    return testudo___implementation::is_valid_get(
      tu, std::make_index_sequence<size>());
  }

  // containers
  template <typename Ca>
  std::enable_if_t<
    testudo___implementation::implementation
      ::support_for_cbegin_cend_v<Ca>,
    bool>
  is_valid_testudo(Ca const &a)
    { return testudo___implementation::is_valid_sequence(a); }

  /// "absdiff()" overloads

  // two tuples or pairs
  template <typename Ta, typename Tb,
            auto size_a=std::tuple_size<Ta>::value,
            auto size_b=std::tuple_size<Tb>::value>
  std::enable_if_t<
    (testudo___implementation::implementation
       ::support_for_tuple_size_v<Ta>
     and testudo___implementation::implementation
           ::support_for_tuple_size_v<Tb>),
    double>
  absdiff_testudo(Ta const &a,
                         Tb const &b) {
    static_assert(size_a==size_b);
    return testudo___implementation::absdiff_get(
      a, b, std::make_index_sequence<size_a>());
  }

  // two containers
  template <typename Ca, typename Cb>
  std::enable_if_t<
    (testudo___implementation::implementation
       ::support_for_cbegin_cend_v<Ca>
     and testudo___implementation::implementation
           ::support_for_cbegin_cend_v<Cb>),
    double>
  absdiff_testudo(Ca const &a, Cb const &b)
    { return testudo___implementation::absdiff_sequence(a, b); }

  /// "to_text()" overloads

  // tuples and pairs
  template <typename Tu,
            auto size=std::tuple_size<Tu>::value>
  std::enable_if_t<
    testudo___implementation::implementation
      ::support_for_tuple_size_v<Tu>,
    void>
  to_stream_testudo(std::ostream &os, Tu const &t) {
    os << "{";
    testudo___implementation::to_stream_get(os, t);
    os << "}";
  }

  // containers
  template <typename Ca>
  std::enable_if_t<
    testudo___implementation::implementation
      ::support_for_cbegin_cend_v<Ca>,
    void>
  to_stream_testudo(std::ostream &os, Ca const &a) {
    os << "{";
    testudo___implementation::to_stream_sequence(os, a);
    os << "}";
  }

}

namespace testudo___implementation {

  /// exceptions
  template <typename E>
  std::string exception_to_text(std::ostream &fmt_os, E const &e) {
    if constexpr (implementation::support_for_what_representation_v<E>) {
      return to_text(fmt_os, e.what());
    }
    else
      return to_text(fmt_os, e);
    }

}

testudo___BRING(is_valid, are_equal, are_approx,
                to_stream, to_text, absdiff, unquoted)

#endif
