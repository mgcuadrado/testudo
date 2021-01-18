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

#ifndef MGCUADRADO_TESTUDO_TEST_OPT_HEADER_
#define MGCUADRADO_TESTUDO_TEST_OPT_HEADER_

#include "testudo_macros.h"
#include <list>
#include <string>

namespace testudo___implementation {

#define main_params                                                \
  [[maybe_unused]] int argc, [[maybe_unused]] char *argv[]
#define main_args argc, argv

  struct opt_t
    : std::string {
    opt_t() : match(false) { }
    opt_t(std::string text) : std::string(text), match(true) { }
    operator bool() const { return match; }
  private:
    bool const match;
  };

  struct opts_t
    : std::list<std::string> {
    opts_t(std::string executable) : executable(executable) { }
    std::string const executable;
    operator bool() const { return not empty(); }
    std::string arg();
    bool opt(std::string o);
    opt_t opt_arg(std::string o);
  };

  opts_t args_to_opts(main_params);

  struct TestOptions {
    TestOptions(opts_t opts);
    std::string format_name;
    std::list<std::string> dynamic_libraries;
    std::string subtree;
    std::list<std::string> include, glob;
  };

  int testudo_main(std::string subtree, main_params);
  inline int testudo_main(main_params) { return testudo_main("", main_args); }

}

testudo___BRING(opts_t, args_to_opts, TestOptions,
                testudo_main)

#endif
