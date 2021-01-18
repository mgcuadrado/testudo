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
#include "testudo_xml_to_color.h"
#include "testudo.h"
#include <iostream>
#include <dlfcn.h>
#include <cassert>

namespace testudo___implementation {

  using namespace std;

  bool opts_t::opt(string o) {
    if (front()==o) {
      pop_front();
      return true;
    }
    else
      return false;
  }

  string opts_t::arg() {
    string result;
    if (not empty()) {
      result=front();
      pop_front();
    }
    return result;
  }

  opt_t opts_t::opt_arg(string o) {
    if (opt(o))
      return {arg()};
    else
      return {};
  }

  opts_t args_to_opts(main_params) {
    opts_t result(argv[0]);
    for (int i=1; i<argc; ++i)
      result.push_back(argv[i]);
    assert(not argv[argc]);
    return result;
  }

  TestOptions::TestOptions(opts_t opts) {
    // this is really the simplest implementation that will do to dectect
    // options; i should probably use a full-fledged option parser
    while (opts) {
      if (auto f=opts.opt_arg("-f"))
        format_name=f;
      else if (auto s=opts.opt_arg("-s"))
        subtree=s;
      else if (auto i=opts.opt_arg("-i"))
        include.push_back(i);
      else if (auto g=opts.opt_arg("-g"))
        glob.push_back(g);
      else if (auto d=opts.opt_arg("-d"))
        TestFormat::location_t::common_directory=d;
      else
        dynamic_libraries.push_back(opts.arg());
    }

    if (format_name=="") {
      cerr << "error; format: "
           << opts. executable << " -f <test_output_format>" << endl;
      exit(1);
    }
  }

  int testudo_main(std::string subtree, main_params) {
    begintrycatch;
    auto opts=testudo::args_to_opts(main_args);
    if (not opts)
      return testudo::testudo_help(), 0;
    auto command=opts.arg();
    if (command=="help")
      return testudo::testudo_help(), 0;
    else if (command=="diff")
      testudo___implementation::testudo_diff(opts);
    else if (command=="xml_to_color")
      testudo___implementation::testudo_xml_to_color(opts);
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
