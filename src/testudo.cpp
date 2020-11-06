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
#include <list>
#include <cassert>

namespace testudo___implementation {

  using namespace std;

  namespace {
    std::string build_full_name(TestNode::pptr parent, name_t name) {
      return (parent and (parent not_eq TestNode::root_node().get())
              ? (parent->full_name+".")
              : std::string())+name;
    }
  }

  TestNode::TestNode(TestFormat::location_t location, pptr parent, name_t name)
    : location(location), parent(parent),
      name(name), full_name(build_full_name(parent, name)) { }

  TestNode::sptr TestNode::get_child(TestFormat::location_t location,
                                     name_t name) {
    if (auto it=children.find(name); it==children.end()) {
      sptr child(new TestNode(location, pptr(this), name));
      children[name]=child;
      return child;
    }
    else
      return it->second;
  }

  void TestNode::check_and_set() {
    if (set_title_and_bucket)
      throw std::runtime_error("test node \""+full_name+"\" set twice");
    set_title_and_bucket=true;
  }

  void TestNode::set(title_t set_title, priority_t set_priority) {
    check_and_set();
    title=set_title;
    priority=set_priority;
  }

  void TestNode::set(title_t set_title, declaration_order_t) {
    check_and_set();
    set_declaration_order=true;
    title=set_title;
    parent->declaration_order_children.push_back(name);
  }

  TestNode::sptr TestNode::root_node() {
    static sptr result(new TestNode({}, nullptr, "/"));
    return result;
  }

  TestNode::sptr TestNode::get_node(std::string const &full_name) {
    if (full_name.empty())
      return root_node();
    else {
      auto last_dot=full_name.find_last_of('.');
      if (last_dot==std::string::npos)
        return make_node({}, "", full_name);
      else {
        auto name=full_name.substr(last_dot+1);
        auto ancestors=full_name.substr(0, last_dot);
        return make_node({}, ancestors, name);
      }
    }
  }

  TestStats
  TestNode::test(test_format_p test_format, string subtree) const {
    bool this_is_root=this==root_node().get();
    if (subtree.empty()) {
      if (this_is_root and children.size()==1)
        return children.begin()->second->test(test_format);
      TestStats test_stats;
      run_tests({test_format, test_stats});
      test_format->print_test_readout();
      return test_stats;
    }
    else {
      if (this_is_root)
        subtree=root_node()->name+"."+subtree;
      auto dot=subtree.find('.');
      if (dot==string::npos) {
        if (subtree==name) // terminal self
          return test(test_format, "");
        else
          return TestStats();
      }

      string rest=subtree.substr(dot+1);
      string child_name=rest.substr(0, rest.find('.'));
      if (auto it=children.find(child_name); it not_eq children.end())
        return it->second->test(test_format, rest);
      else
        return TestStats();
    }
  }

  list<TestNode::sptr> TestNode::ordered_children() const {
    list<sptr> ordered_children;

    // declaration-ordered children
    for (auto const &child_name: declaration_order_children) {
      sptr child=children.at(child_name);
      assert(child);
      assert(child->set_declaration_order);
      ordered_children.push_back(child);
    }

    // prioritised children: first by priority, then alphabetically
    map<priority_t, map<name_t, sptr>> sorting;
    for (auto const &[name, child]: children) {
      assert(child);
      if (not child->set_declaration_order)
        sorting[child->priority][child->name]=child;
    }
    for (auto const &[priority, priority_map]: sorting)
      for (auto const &[name, child]: priority_map)
        ordered_children.push_back(child);

    return ordered_children;
  }

  void TestNode::run_tests(test_management_t test_management) const {
    test_management.format->output_location_title(location, full_name, title);

    // run the children's tests in order
    for (auto const &child: ordered_children()) {
      TestStats child_test_stats;
      child->run_tests({test_management.format, child_test_stats});
      test_management.stats+=child_test_stats;
    }

    // run own test function if set
    if (test_f) {
      try {
        test_f(test_management);
      }
      catch (exception const &excp) {
        test_management.format->uncaught_exception(excp.what());
        test_management.stats.unexpected_error();
      }
      catch (char const *mess) {
        test_management.format->uncaught_exception(mess);
        test_management.stats.unexpected_error();
      }
      catch (...) {
        test_management.format->uncaught_exception("<unknown error type>");
        test_management.stats.unexpected_error();
      }
    }

    test_management.format
      ->produce_summary(full_name, test_management.stats);
  }

  namespace {

    void print_tree_r(ostream &os, TestNode::csptr node,
                      string prefix="", string last_prefix="",
                      TestNode::csptr parent={},
                      TestNode::csptr last_sibling={}) {
      auto ordered_children=node->ordered_children();
      if (node==TestNode::root_node() and ordered_children.size()==1)
        return print_tree_r(os, ordered_children.front());

      bool last_child=(node==last_sibling);
      string line_prefix=parent ? (last_child ? "`-" : "|-") : "--";
      os << prefix << line_prefix << node->name << "\n";
      string next_last_prefix=last_child ? "   " : last_prefix;
      for (auto const &child: ordered_children)
        print_tree_r(os, child,
                     prefix+next_last_prefix, "|  ",

                     node, *ordered_children.rbegin());
    }

  }

  void print_tree(ostream &os, TestNode::csptr node)
    { print_tree_r(os, node); }

}
