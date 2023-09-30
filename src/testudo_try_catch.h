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

#ifndef MGCUADRADO_TESTUDO_TRY_CATCH_HEADER_
#define MGCUADRADO_TESTUDO_TRY_CATCH_HEADER_

#include <iostream>
#include <exception>

namespace testudo___implementation {

#define mgcuadrado_testudo_eat_a_semicolon static_cast<void>(0)
#define mgcuadrado_testudo_decl_eat_a_semicolon                              \
  struct never_define_this_struct_68e47e1831e0eebbe5994ad57527f71b

#define begintrycatch try { mgcuadrado_testudo_eat_a_semicolon
#define endtrycatch } \
  catch (std::exception const &excp)                                         \
    { std::cerr << std::endl << "Exception: " << excp.what() << std::endl; } \
  catch (char const *mess)                                                   \
    { std::cerr << std::endl << "Error: " << mess << std::endl; }            \
  catch (...)                                                                \
    { std::cerr << std::endl << "Uncatched error" << std::endl; throw; }     \
    mgcuadrado_testudo_eat_a_semicolon

}

#endif
