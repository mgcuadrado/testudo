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
