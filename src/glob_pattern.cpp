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

#include "glob_pattern.h"
#include <stdexcept>

namespace glob {

  using namespace std;

  bool matches(string name, string pattern) {
    size_t n=0, p=0; // "n": name index; "p": pattern index
    size_t const n_s=name.size(), p_s=pattern.size();
    while (p<p_s) {
      char c=pattern[p];
      ++p;
      switch (c) {
        case '\\': {
          if (p<p_s) {
            char c=pattern[p];
            ++p;
            switch(c) {
              case '?': case '*': case '\\': {
                if (n<n_s and name[n]==c)
                  ++n;
                else
                  return false;
              } break;
              default: {
                throw runtime_error("wrong wildcard at char "+to_string(p));
              } break;
            }
          }
          else
            throw runtime_error("wrong wildcard at char "+to_string(p));
        } break;
        case '?': {
          if (n<n_s)
            ++n;
          else
            return false;
        } break;
        case '*': {
          string rest=pattern.substr(p); // past the '*'
          for (size_t i=n; i<=n_s; ++i)
            if (matches(name.substr(i), rest))
              return true;
          return false;
        } break;
        default: {
          if (n<n_s and name[n]==c)
            ++n;
          else
            return false;
        } break;
      }
    }
    // pattern exhausted; match only if name also exhausted
    return n==n_s;
  }

}
