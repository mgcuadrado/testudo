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

#define testudo___FOR_EACH_00(WHAT)
#define testudo___FOR_EACH_01(WHAT, X)      \
  WHAT(X)
#define testudo___FOR_EACH_02(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_01(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_03(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_02(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_04(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_03(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_05(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_04(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_06(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_05(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_07(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_06(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_08(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_07(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_09(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_08(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_10(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_09(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_11(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_10(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_12(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_11(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_13(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_12(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_14(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_13(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_15(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_14(WHAT, __VA_ARGS__)
#define testudo___FOR_EACH_16(WHAT, X, ...) \
  WHAT(X)testudo___FOR_EACH_15(WHAT, __VA_ARGS__)
//... repeat as needed

#define testudo___GET_MACRO(ACTION,                                     \
                            A01, A02, A03, A04, A05, A06, A07, A08,     \
                            A09, A10, A11, A12, A13, A14, A15, A16,     \
                            NAME, ...) NAME
// the action is the first argument:
#define testudo___FOR_EACH(...)                                         \
  testudo___GET_MACRO(                                                  \
    __VA_ARGS__,                                                        \
    testudo___FOR_EACH_16, testudo___FOR_EACH_15,                       \
    testudo___FOR_EACH_14, testudo___FOR_EACH_13,                       \
    testudo___FOR_EACH_12, testudo___FOR_EACH_11,                       \
    testudo___FOR_EACH_10, testudo___FOR_EACH_09,                       \
    testudo___FOR_EACH_08, testudo___FOR_EACH_07,                       \
    testudo___FOR_EACH_06, testudo___FOR_EACH_05,                       \
    testudo___FOR_EACH_04, testudo___FOR_EACH_03,                       \
    testudo___FOR_EACH_02, testudo___FOR_EACH_01,                       \
    testudo___FOR_EACH_00,                                              \
                      )(__VA_ARGS__)

#define testudo___BRING_ONE(name)                                       \
  namespace testudo { using testudo___implementation::name; }
#define testudo___BRING(...) \
  testudo___FOR_EACH(testudo___BRING_ONE, __VA_ARGS__)

#endif
