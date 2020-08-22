#ifndef MGCUADRADO_TESTUDO_EXT_HEADER_
#define MGCUADRADO_TESTUDO_EXT_HEADER_

// Testudo support for some STL objects: "absdiff()" and "to_text()" overloads.

#include "testudo_base.h"
#include <tuple>
#include <utility>
#include <vector>
#include <array>
#include <list>
#include <stdexcept>

namespace testudo::implementation { // FIXME: rename to "testudo"

  template <typename A, typename B, std::size_t... i>
  double absdiff_get(A const &a, B const &b, std::index_sequence<i...>)
    { return (testudo::absdiff(std::get<i>(a), std::get<i>(b))+...); }

  template <typename A, typename B>
  double absdiff_sequence(A const &a, B const &b) {
    if (a.size() not_eq b.size())
      throw std::invalid_argument("incompatible sizes in \"absdiff()\"");
    double result=0.;
    auto ai=a.begin();
    auto bi=b.begin();
    for (std::size_t i=0; i<a.size(); ++i)
      result+=absdiff(*(ai++), *(bi++));
    return result;
  }

  template <typename A, std::size_t... i>
  std::string to_text_get(A const &a, std::index_sequence<i...>)
    { return (((i==0 ? "" : ", ")+testudo::to_text(std::get<i>(a)))+...); }

  template <typename A>
  std::string to_text_sequence(A const &a) {
    if (a.empty())
      return "";
    else {
      std::string result=to_text(a.front());
      for (auto i=std::next(a.begin()); i not_eq a.end(); ++i)
        result+=", "+to_text(*i);
      return result;
    }
  }

}

namespace std {
  // overloads are in the "std" namespace, where the types they apply to are
  // defined, so they're found by argument-dependent lookup

  /// "absdiff()" overloads

  // two tuples
  template <typename... Ta, typename... Tb>
  double absdiff(std::tuple<Ta...> const &a, std::tuple<Tb...> const &b) {
    static_assert(sizeof...(Ta)==sizeof...(Tb));
    return testudo::implementation::absdiff_get(
      a, b, std::index_sequence_for<Ta...>());
  }

  // two pairs
  template <typename Ta1, typename Ta2, typename Tb1, typename Tb2>
  double absdiff(std::pair<Ta1, Ta2> const &a, std::pair<Tb1, Tb2> const &b) {
    return testudo::implementation::absdiff_get(
      a, b, std::make_index_sequence<2>());
  }

  // two vectors
  template <typename Ta, typename Aa, typename Tb, typename Ab>
  double absdiff(std::vector<Ta, Aa> const &a, std::vector<Tb, Ab> const &b)
    { return testudo::implementation::absdiff_sequence(a, b); }

  // two arrays
  template <typename Ta, std::size_t Na, typename Tb, std::size_t Nb>
  double absdiff(std::array<Ta, Na> const &a, std::array<Tb, Nb> const &b)
    { return testudo::implementation::absdiff_sequence(a, b); }

  // two lists
  template <typename Ta, typename Aa, typename Tb, typename Ab>
  double absdiff(std::list<Ta, Aa> const &a, std::list<Tb, Ab> const &b)
    { return testudo::implementation::absdiff_sequence(a, b); }

  /// "to_text()" overloads

  // tuple
  template <typename... T>
  std::string to_text(std::tuple<T...> const &t) {
    return
      "tuple{"
      +testudo::implementation::to_text_get(t, std::index_sequence_for<T...>())
      +"}";
  }

  // pair
  template <typename T1, typename T2>
  std::string to_text(std::pair<T1, T2> const &p) {
    return
      "pair{"
      +testudo::implementation::to_text_get(p, std::make_index_sequence<2>())
      +"}";
  }

  // vector
  template <typename T, typename A>
  std::string to_text(std::vector<T, A> const &v)
    { return "vector{"+testudo::implementation::to_text_sequence(v)+"}"; }

  // array
  template <typename T, std::size_t N>
  std::string to_text(std::array<T, N> const &v)
    { return "array{"+testudo::implementation::to_text_sequence(v)+"}"; }

  // list
  template <typename T, typename A>
  std::string to_text(std::list<T, A> const &v)
    { return "list{"+testudo::implementation::to_text_sequence(v)+"}"; }

}

#endif
