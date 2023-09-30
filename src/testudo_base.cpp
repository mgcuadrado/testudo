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

#include "testudo_base.h"

namespace testudo___implementation {

  using namespace std;

  ostringstream default_fmt_os;

  static bool default_fmt_os_initialized=
    []() { default_fmt_os << boolalpha; return true; }();

  bool no_quotes_around_strings=false;

  void set_no_quotes_around_string() { no_quotes_around_strings=true; }
  void unset_no_quotes_around_string() { no_quotes_around_strings=false; }

}
