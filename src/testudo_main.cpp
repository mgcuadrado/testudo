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

#include "testudo.h"
#include "testudo_opt.h"
#include "testudo_try_catch.h"
#include <dlfcn.h>

int main(main_opts_params) {
  begintrycatch;
  testudo::TestOptions to(main_opts_args);
  for (std::string library: to.dynamic_libraries) {
    dlopen(library.c_str(), RTLD_LAZY);
    if (auto error=dlerror()) {
      std::cerr << "dlopen() error opening \""+library+"\": "
                << error << std::endl;
      return 1;
    }
  }
  testudo::TestNode::root_node()
    ->test(testudo::test_format_named_creator(to.format_name)(std::cout),
           to.subtree);
  endtrycatch;
}