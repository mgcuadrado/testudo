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

#ifndef MGCUADRADO_TESTUDO_FORMAT_XML_HEADER_
#define MGCUADRADO_TESTUDO_FORMAT_XML_HEADER_

#include <string>

namespace testudo___implementation {

  // replace all "~" instances with "||~tilde~||~"; codes in "||~"..."~||"
  // are assumed to have no length, so we add "~" after "||~tilde~||" so that
  // it contributes to the computed length as what it's going to be; we'll
  // have to simply remove all "||~tilde~||" instances at the end
  std::string escape_tilde(std::string);

  // inverse of "escape_tilde()"
  std::string unescape_tilde(std::string);

}

#endif
