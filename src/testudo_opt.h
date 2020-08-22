#ifndef TESTIT_TEST_OPT_HEADER_
#define TESTIT_TEST_OPT_HEADER_

#include <list>
#include <string>
#include <iostream>
#include <algorithm>
#include <cassert>

namespace testudo {

#define main_opts_params                                                \
  [[maybe_unused]] int argc, [[maybe_unused]] char *argv[]
#define main_opts_args argc, argv

  using opts_t=std::list<std::string>;

  inline opts_t to_list(main_opts_params) {
    opts_t result;
    for (int i=0; i<argc; ++i)
      result.push_back(argv[i]);
    assert(not argv[argc]);
    return result;
  }

  struct TestOptions {
    TestOptions(opts_t opts) {
      // this is really the simplest implementation that will do to dectect
      // options; i should probably use a full-fledged option parser
      for (auto i=opts.begin(); i not_eq opts.end(); ++i) {
        if (*i=="-f" and ++i not_eq opts.end())
          format_name=*i;
        else if (*i=="-s" and ++i not_eq opts.end())
          subtree=*i;
      }

      if (format_name=="") {
        std::cerr << "error; format: " << opts.front()
                  << " -f <test_output_format>" << std::endl;
        exit(1);
      }
    }
    TestOptions(main_opts_params)
      : TestOptions(to_list(main_opts_args)) { }
    std::string format_name;
    std::string subtree;
  };

}

#endif
