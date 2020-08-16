#include "testudo_lc.h"
#include "testudo_opt.h"
#include "testudo_try_catch.h"
#include <sstream>
#include <iostream>

using namespace std;

namespace {

  define_top_test_node("", testudo, "Testudo");
  define_top_test_node("testudo", main, "main() test");
  define_test(main, show_tree, "show test tree") {
    declare(ostringstream trs);
    perform(testudo::print_tree(trs, testudo::TestNode::root_node()));
    show_multiline_value(trs.str());
  }

}

int main(main_opts_params) {
  begintrycatch;
  testudo::TestOptions to(main_opts_args);
  top_test_node("testudo")
    ->test(testudo::test_format_named_creator(to.format_name)(cout),
           to.subtree);
  endtrycatch;
}
