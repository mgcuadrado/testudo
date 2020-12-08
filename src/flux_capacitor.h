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

#ifndef TESTUDO_OUTATIME_FLUX_CAPACITOR_HEADER_
#define TESTUDO_OUTATIME_FLUX_CAPACITOR_HEADER_

class Delorean;

class FluxCapacitor {
public:
  FluxCapacitor()
    : on(false), charged_jw(0.) { }
  bool is_on() const { return on; }
  double missing_jw() const { return required_jw-charged_jw; }
  void connect_to(Delorean *) { } // not implemented yet
private:
  bool on;
  double const required_jw=2.21;
  double charged_jw;
};

#endif
