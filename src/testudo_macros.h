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

#ifndef MGCUADRADO_TESTUDO_MACROS_HEADER_
#define MGCUADRADO_TESTUDO_MACROS_HEADER_

#include "testudo_macros_n.gh"

#define testudo___BRING_ONE(name)                                       \
  namespace testudo { using testudo___implementation::name; }
#define testudo___BRING(...) \
  testudo___FOR_EACH(testudo___BRING_ONE, __VA_ARGS__)


#define testudo___CAT(a, b) testudo___CAT_IMPL(a, b)
#define testudo___CAT_IMPL(a, b) a ## b
#define testudo___STRING(a) testudo___STRING_IMPL(a)
#define testudo___STRING_IMPL(a) #a

#define testudo___TAKE_SINGLE(_1) _1
#define testudo___TAKE_1ST(_1, _2) _1
#define testudo___TAKE_2ND(_1, _2) _2
  // SELECT_1((first, second)) -> first
#define testudo___SELECT_1(...) \
  testudo___SELECT_1_IMPL(testudo___REMOVE_BRACKETS __VA_ARGS__)
#define testudo___SELECT_1_IMPL(...) \
  testudo___TAKE_1ST(__VA_ARGS__)
  // SELECT_2((first, second)) -> second
#define testudo___SELECT_2(...) \
  testudo___SELECT_2_IMPL(testudo___REMOVE_BRACKETS __VA_ARGS__)
#define testudo___SELECT_2_IMPL(...) \
  testudo___TAKE_2ND(__VA_ARGS__)

#define testudo___IF_BRACKETS_1(...) 1
#define testudo___IF_BRACKETS_2(...) 2
#define testudo___SELECT_testudo___IF_BRACKETS_1 testudo___TAKE_SINGLE,
#define testudo___SELECT_testudo___IF_BRACKETS_2 testudo___TAKE_SINGLE,

#define testudo___ARG1(_1, ...) _1
#define testudo___EXPAND_ARG1(...) testudo___ARG1(__VA_ARGS__, )
  // SINGLE_OR_1ST(single) -> single
  // SINGLE_OR_1ST((first, second)) -> first
#define testudo___SINGLE_OR_1ST(a)                                      \
  testudo___EXPAND_ARG1(                                                \
    testudo___CAT(testudo___SELECT_,                                    \
                  testudo___IF_BRACKETS_1 a))(a)
  // SINGLE_OR_1ST(single) -> single
  // SINGLE_OR_1ST((first, second)) -> second
#define testudo___SINGLE_OR_2ND(a)                                      \
  testudo___EXPAND_ARG1(                                                \
    testudo___CAT(testudo___SELECT_,                                    \
                  testudo___IF_BRACKETS_2 a))(a)

#define testudo___FIRST(...) testudo___FIRST_IMPL(__VA_ARGS__, )
#define testudo___FIRST_IMPL(a, ...) a

#define testudo___DECL_EAT_A_SEMICOLON                                  \
  struct never_define_this_struct_68e47e1831e0eebbe5994ad57527f71b

#endif
