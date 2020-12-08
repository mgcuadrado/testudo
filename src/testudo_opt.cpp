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

#include "testudo_opt.h"
#include "testudo_try_catch.h"
#include "testudo_help.h"
#include "testudo_diff.h"
#include "testudo.h"
#include <iostream>
#include <dlfcn.h>
#include <cassert>

namespace testudo___implementation {

  using namespace std;

  opts_t args_to_opts(main_params) {
    opts_t result(argv[0]);
    for (int i=1; i<argc; ++i)
      result.push_back(argv[i]);
    assert(not argv[argc]);
    return result;
  }

  string front_and_shift(opts_t &o) {
    auto result=o.front();
    o.pop_front();
    return result;
  }

  TestOptions::TestOptions(opts_t opts) {
    // this is really the simplest implementation that will do to dectect
    // options; i should probably use a full-fledged option parser
    for (auto i=opts.begin(); i not_eq opts.end(); ++i) {
      if ((*i=="-f") and (++i not_eq opts.end()))
        format_name=*i;
      else if ((*i=="-s") and (++i not_eq opts.end()))
        subtree=*i;
      else if ((*i=="-i") and (++i not_eq opts.end()))
        include.push_back(*i);
      else if ((*i=="-g") and (++i not_eq opts.end()))
        glob.push_back(*i);
      else if ((*i=="-d") and (++i not_eq opts.end()))
        TestFormat::location_t::common_directory=*i;
      else
        dynamic_libraries.push_back(*i);
    }

    if (format_name=="") {
      cerr << "error; format: " << opts.front() << " -f <test_output_format>"
           << endl;
      exit(1);
    }
  }

  int testudo_main(std::string subtree, main_params) {
    begintrycatch;
    auto opts=testudo::args_to_opts(main_args);
    if (opts.size()==0)
      return testudo::testudo_help(), 0;
    auto command=testudo::front_and_shift(opts);
    if (command=="help")
      return testudo::testudo_help(), 0;
    else if (command=="diff")
      testudo::testudo_diff(opts);
    else if (command=="run") {
      testudo::TestOptions to(opts);
      for (string library: to.dynamic_libraries) {
        dlopen(library.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (auto error=dlerror()) {
          cerr << "dlopen() error opening \""+library+"\": "
                    << error << endl;
          return 2;
        }
      }
      auto test_root=
        testudo__TOP_TEST_NODE(to.subtree.empty() ? subtree : to.subtree);
      if  (test_root)
        test_root
          ->test(testudo::test_format_named_creator(to.format_name)(cout),
                 to.include, to.glob);
      else {
        cerr << opts.executable << ": unknown node \"" << to.subtree << "\""
                  << endl;
        return 1;
      }
    }
    else {
      cerr << opts.executable << ": unknown command \"" << command << "\""
           << endl;
      return 1;
    }
    endtrycatch;

    return 0;
  }

}
