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

#ifndef TESTUDO_OUTATIME_DELOREAN_TEST_HEADER_
#define TESTUDO_OUTATIME_DELOREAN_TEST_HEADER_

#include <testudo/testudo.h> // default lower-case style

class Delorean;

struct DeloreanFixture
  : testudo::Fixture {
  using Fixture::Fixture;
  Delorean *delorean=nullptr;
};

#endif
