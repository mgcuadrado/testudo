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

#ifndef MGCUADRADO_TRACE_
#define MGCUADRADO_TRACE_

#ifdef DO_TRACE

#include <iostream>

inline std::ostream &trace=std::cout; // FIXME: make this settable by user?
using std::endl;

#define DEFINE_TRACE(name)                                              \
  inline unsigned trace___TRACE_VARIABLE_ ## name=0
#define LOCAL_TRACE(name)                                               \
  unsigned const &trace___LOCAL_TRACE_VARIABLE=                         \
    trace___TRACE_VARIABLE_ ## name
#define SET_TRACE(name) trace___TRACE_VARIABLE_ ## name=1
#define SET_TRACE_2(name) trace___TRACE_VARIABLE_ ## name=2
#define SET_TRACE_3(name) trace___TRACE_VARIABLE_ ## name=3
#define UNSET_TRACE(name) trace___TRACE_VARIABLE_ ## name=0

#define TRACE(name, ...)                                                \
  if (trace___TRACE_VARIABLE_ ## name) __VA_ARGS__
#define TRACE_2(name, ...)                                              \
  if (trace___TRACE_VARIABLE_ ## name>=2) __VA_ARGS__
#define TRACE_3(name, ...)                                              \
  if (trace___TRACE_VARIABLE_ ## name>=3) __VA_ARGS__
#define LTRACE(...)                                                     \
  if (trace___LOCAL_TRACE_VARIABLE) __VA_ARGS__
#define LTRACE_2(...)                                                   \
  if (trace___LOCAL_TRACE_VARIABLE>=2) __VA_ARGS__
#define LTRACE_3(...)                                                   \
  if (trace___LOCAL_TRACE_VARIABLE>=3) __VA_ARGS__

#else

#define DEFINE_TRACE(name)                                              \
  struct never_define_this_struct_68e47e1831e0eebbe5994ad57527f71b
#define LOCAL_TRACE(name) (void) 0
#define SET_TRACE(name) (void) 0
#define SET_TRACE_2(name) (void) 0
#define SET_TRACE_3(name) (void) 0
#define UNSET_TRACE(name) (void) 0

#define TRACE(name, ...) (void) 0
#define TRACE_2(name, ...) (void) 0
#define TRACE_3(name, ...) (void) 0
#define LTRACE(...) (void) 0
#define LTRACE_2(...) (void) 0
#define LTRACE_3(...) (void) 0

#endif

#endif
