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

#ifndef MGCUADRADO_TESTUDO_EXT_HEADER_
#define MGCUADRADO_TESTUDO_EXT_HEADER_

// Testudo support for some STL objects: "absdiff()" and "to_text()" overloads.

#include "testudo_base.h"
#include <map>
#include <tuple>
#include <utility>
#include <vector>
#include <array>
#include <list>
#include <stdexcept>

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
    if (a.size() not_eq b.size())
      throw std::invalid_argument("incompatible sizes in \"absdiff()\"");
    double result=0.;
    auto ai=a.begin();
    auto bi=b.begin();
    for (std::size_t i=0; i<a.size(); ++i)
      result+=absdiff(*(ai++), *(bi++));
    return result;
  }

  template <typename A>
  std::string to_text_get(A const &, std::index_sequence<>)
    { return ""; }
  template <typename A, std::size_t i>
  std::string to_text_get(A const &a, std::index_sequence<i>)
    { return to_text(std::get<i>(a)); }
  template <typename A, std::size_t i, std::size_t... j>
  std::string to_text_get(A const &a, std::index_sequence<i, j...>) {
    return to_text_get(a, std::index_sequence<i>())
      +", "+to_text_get(a, std::index_sequence<j...>());
  }
  template <typename... A>
  std::string to_text_get(std::tuple<A...> const &t)
    { return to_text_get(t, std::index_sequence_for<A...>()); }

  template <typename A>
  std::string to_text_sequence(A const &a) {
    if (a.empty())
      return "";
    else {
      bool first=true;
      std::string result;
      for (auto const &e: a) {
        result+=(first ? "": ", ")+to_text(e);
        first=false;
      }
      return result;
    }
  }

  template <typename K, typename T, typename C, typename A>
  std::string to_text_sequence(std::map<K, T, C, A> const &a) {
    if (a.empty())
      return "";
    else {
      bool first=true;
      std::string result;
      for (auto const &e: a) {
        result+=
          std::string(first ? "": ", ")
          +"{"+to_text(e.first)+", "+to_text(e.second)+"}";
        first=false;
      }
      return result;
    }
  }

}

namespace std {
  // overloads are in the "std" namespace, where the types they apply to are
  // defined, so they're found by argument-dependent lookup

  /// "is_valid()" overloads

  // two tuples
  template <typename... Ta>
  bool is_valid_testudo(std::tuple<Ta...> const &a) {
    return testudo___implementation::is_valid_get(
      a, std::index_sequence_for<Ta...>());
  }

  // two pairs
  template <typename Ta1, typename Ta2>
  bool is_valid_testudo(std::pair<Ta1, Ta2> const &a) {
    return testudo___implementation::is_valid_get(
      a, std::make_index_sequence<2>());
  }

  // two vectors
  template <typename Ta, typename Aa>
  bool is_valid_testudo(std::vector<Ta, Aa> const &a)
    { return testudo___implementation::is_valid_sequence(a); }

  // two arrays
  template <typename Ta, std::size_t Na>
  bool is_valid_testudo(std::array<Ta, Na> const &a)
    { return testudo___implementation::is_valid_sequence(a); }

  // two lists
  template <typename Ta, typename Aa>
  bool is_valid_testudo(std::list<Ta, Aa> const &a)
    { return testudo___implementation::is_valid_sequence(a); }

  /// "absdiff()" overloads

  // two tuples
  template <typename... Ta, typename... Tb>
  double absdiff_testudo(std::tuple<Ta...> const &a,
                         std::tuple<Tb...> const &b) {
    static_assert(sizeof...(Ta)==sizeof...(Tb));
    return testudo___implementation::absdiff_get(
      a, b, std::index_sequence_for<Ta...>());
  }

  // two pairs
  template <typename Ta1, typename Ta2, typename Tb1, typename Tb2>
  double absdiff_testudo(std::pair<Ta1, Ta2> const &a,
                         std::pair<Tb1, Tb2> const &b) {
    return testudo___implementation::absdiff_get(
      a, b, std::make_index_sequence<2>());
  }

  // two vectors
  template <typename Ta, typename Aa, typename Tb, typename Ab>
  double absdiff_testudo(std::vector<Ta, Aa> const &a,
                         std::vector<Tb, Ab> const &b)
    { return testudo___implementation::absdiff_sequence(a, b); }

  // two arrays
  template <typename Ta, std::size_t Na, typename Tb, std::size_t Nb>
  double absdiff_testudo(std::array<Ta, Na> const &a,
                         std::array<Tb, Nb> const &b)
    { return testudo___implementation::absdiff_sequence(a, b); }

  // two lists
  template <typename Ta, typename Aa, typename Tb, typename Ab>
  double absdiff_testudo(std::list<Ta, Aa> const &a,
                         std::list<Tb, Ab> const &b)
    { return testudo___implementation::absdiff_sequence(a, b); }

  /// "to_text()" overloads

  // tuple
  template <typename... T>
  std::string to_text_testudo(std::tuple<T...> const &t,
                              std::string prefix="tuple") {
    return
      prefix+"{"
      +testudo___implementation::to_text_get(t)
      +"}";
  }

  // pair
  template <typename T1, typename T2>
  std::string to_text_testudo(std::pair<T1, T2> const &p) {
    return
      "pair{"
      +testudo___implementation::to_text_get(p, std::make_index_sequence<2>())
      +"}";
  }

  // vector
 template <typename T, typename A>
  std::string to_text_testudo(std::vector<T, A> const &v)
    { return "vector{"+testudo___implementation::to_text_sequence(v)+"}"; }

  // array
  template <typename T, std::size_t N>
  std::string to_text_testudo(std::array<T, N> const &v)
    { return "array{"+testudo___implementation::to_text_sequence(v)+"}"; }

  // list
  template <typename T, typename A>
  std::string to_text_testudo(std::list<T, A> const &v)
    { return "list{"+testudo___implementation::to_text_sequence(v)+"}"; }

  // map
  template <typename K, typename T, typename C, typename A>
  std::string to_text_testudo(std::map<K, T, C, A> const &v)
    { return "map{"+testudo___implementation::to_text_sequence(v)+"}"; }

}

#endif
