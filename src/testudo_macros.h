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

#ifndef MGCUADRADO_TESTUDO_MACROS_HEADER_
#define MGCUADRADO_TESTUDO_MACROS_HEADER_

#include "testudo_macros_n.gh"

#define testudo___BRING(...) \
  testudo___FOR_EACH(testudo___BRING_ONE, __VA_ARGS__)
// implementation
#define testudo___BRING_ONE(name)                                       \
  namespace testudo { using testudo___implementation::name; }

// EXPAND(...) -> expanded(...)
#define testudo___EXPAND(...) __VA_ARGS__

// STRING(a) -> # expanded(a)
#define testudo___STRING(...) testudo___STRING_EXPANDED(__VA_ARGS__)
// implementation
#define testudo___STRING_EXPANDED(...) #__VA_ARGS__

// CAT(a, b) -> expanded(a) ## expanded(b)
#define testudo___CAT(a, b) testudo___CAT_EXPANDED(a, b)
// implementation
#define testudo___CAT_EXPANDED(a, b) a ## b

// if expanded(a) is (b), REMOVE_BRACKETS a -> b
#define testudo___REMOVE_BRACKETS(...) __VA_ARGS__

// TAKE_SINGLE(single) -> single
#define testudo___TAKE_SINGLE(_1) _1
// TAKE_1ST(first, second) -> first
#define testudo___TAKE_1ST(_1, _2) _1
// TAKE_2ND(first, second) -> second
#define testudo___TAKE_2ND(_1, _2) _2
// SELECT_1((first, second)) -> first

// SINGLE_OR_1ST(single) -> expand(single)
// SINGLE_OR_1ST((first, second)) -> expand(first)
#define testudo___SINGLE_OR_1ST(a)                                      \
  testudo___EXPAND_ARG1(                                                \
    testudo___CAT(testudo___SELECT_IMPL_,                               \
                  testudo___IF_BRACKETS_1 a))(a)
// SINGLE_OR_2nd(single) -> expand(single)
// SINGLE_OR_2nd((first, second)) -> expand(second)
#define testudo___SINGLE_OR_2ND(a)                                      \
  testudo___EXPAND_ARG1(                                                \
    testudo___CAT(testudo___SELECT_IMPL_,                               \
                  testudo___IF_BRACKETS_2 a))(a)
// implementation
#define testudo___EXPAND_ARG1(...) testudo___ARG1(__VA_ARGS__, )
#define testudo___ARG1(_1, ...) _1
#define testudo___SELECT_IMPL_1(...)                                    \
  testudo___SELECT_1_EXPANDED(testudo___REMOVE_BRACKETS __VA_ARGS__)
#define testudo___SELECT_1_EXPANDED(...)                                \
  testudo___TAKE_1ST(__VA_ARGS__)
#define testudo___SELECT_IMPL_2(...)                                    \
  testudo___SELECT_2_EXPANDED(testudo___REMOVE_BRACKETS __VA_ARGS__)
#define testudo___SELECT_2_EXPANDED(...)                                \
  testudo___TAKE_2ND(__VA_ARGS__)
#define testudo___IF_BRACKETS_1(...) 1
#define testudo___IF_BRACKETS_2(...) 2
#define testudo___SELECT_IMPL_testudo___IF_BRACKETS_1 testudo___TAKE_SINGLE,
#define testudo___SELECT_IMPL_testudo___IF_BRACKETS_2 testudo___TAKE_SINGLE,

// FIRST(a, ...) -> expand(a)
#define testudo___FIRST(...) testudo___FIRST_IMPL(__VA_ARGS__, )
// implementation
#define testudo___FIRST_IMPL(a, ...) a

  // IS_EMPTY() -> 1
  // IS_EMPTY(a) -> 0
  // IS_EMPTY(a, ...) -> 0
  // "testudo___IS_EMPTY()" adapted from
  // https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments
#define testudo___IS_EMPTY(...)                                         \
  testudo___IS_EMPTY_IMPL(                                              \
    /* test if there is just one argument, eventually an empty one */   \
    testudo___HAS_COMMA(__VA_ARGS__),                                   \
    /* test if TRIGGER_PARENTHESIS_ together with the argument adds a   \
       comma */                                                         \
    testudo___HAS_COMMA(testudo___TRIGGER_PARENTHESIS_ __VA_ARGS__),    \
    /* test if the argument together with a parenthesis adds a comma */ \
    testudo___HAS_COMMA(__VA_ARGS__ (/*empty*/)),                       \
    /* test if placing it between TRIGGER_PARENTHESIS_ and the          \
       parenthesis adds a comma */                                      \
    testudo___HAS_COMMA(                                                \
      testudo___TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/))           \
  )
  // implementation
#define testudo___TRIGGER_PARENTHESIS_(...) ,
#define testudo___PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define testudo___IS_EMPTY_IMPL(_0, _1, _2, _3)                          \
  testudo___HAS_COMMA(testudo___PASTE5(                                 \
    testudo___IS_EMPTY_CASE_, _0, _1, _2, _3))
#define testudo___IS_EMPTY_CASE_0001 ,

#define testudo___COMMA_IF_NOT_EMPTY(...)                               \
  testudo___CAT(testudo___COMMA_IF_NOT_EMPTY_,                          \
                testudo___IS_EMPTY(__VA_ARGS__))
  // implementation
#define testudo___COMMA_IF_NOT_EMPTY_0 ,
#define testudo___COMMA_IF_NOT_EMPTY_1

// DECL_EAT_A_SEMICOLON eats an unwanted semicolon after a declaration
#define testudo___DECL_EAT_A_SEMICOLON                                  \
  struct never_define_this_struct_68e47e1831e0eebbe5994ad57527f71b

#endif
