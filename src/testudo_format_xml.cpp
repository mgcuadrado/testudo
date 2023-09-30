// Copyright © 2020-2023 Miguel González Cuadrado <mgcuadrado@gmail.com>

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

#include "testudo_format.h"
#include "kmsxml.h"
#include <ostream>
#include <regex>

using namespace testudo;
using namespace kmsxml;
using namespace std;

namespace testudo___implementation {

  string escape_tilde(string s)
    { return regex_replace(s, regex("~"), "||~tilde~||~"); }

  string unescape_tilde(string s)
    { return regex_replace(s, regex("\\|\\|~tilde~\\|\\|~"), "~"); }

}

namespace {

  using testudo___implementation::escape_tilde;

  string xml_bool(bool b) {
    return
      b ? testudo___implementation::true_tag
      : testudo___implementation::false_tag;
  }
  string xml_bool(string b) { return b; }

  class TestFormatXML
    : public TestFormat {
  public:
    element_t::root_t root{element_t::make_root("testudo")};

  private:
    // since we output XML as we "create" the tree, we don't actually need to
    // create the tree; therefore, we create the top element in each
    // "output_...()" method as a tree root, rather than as a node of an
    // existing tree; that's why the stack needs to keep roots (shared
    // pointers) rather than nodes (weak shared pointers)
    stack<element_t::root_t> test_stack;
    std::size_t test_depth() const { return test_stack.size()-1; }
  public:
    element_t::node_t current_test() { return test_stack.top(); }
    // after each output, we flush the output in case a crash happens
    void push_open(element_t::root_t t) {
      test_stack.push(t);
      current_test()->output_begin(ts, test_depth(), true);
      ts << flush;
    }
    void close_pop() {
      current_test()->output_end(ts, test_depth(), true);
      ts << flush;
      test_stack.pop();
    }
    void print(element_t::node_t t) {
      t->output(ts, test_depth(), false);
      ts << flush;
    }

    ostream &ts;
    TestFormatXML(ostream &os) : ts(os) { push_open(root); }
    ~TestFormatXML() {
      close_pop();
    }

    template <typename E> // E is element::root_t or element::node_t
    E localize(E element) {
      element
        ->append_attribute("location", get_location())
        ->append_attribute("brief_location", get_brief_location());
      set_location({});
      return element;
    }

    void output_title(string name, string title) override {
      auto test_element=element_t::make_root("test");
      test_element
        ->append_attribute("location", get_title_location())
        ->append_attribute("name", escape_tilde(name))
        ->append_attribute("title", escape_tilde(title));
      push_open(test_element);
    }

    void output_begin_indent() override {
      auto indent_element=localize(element_t::make_root("indent"));
      push_open(indent_element);
    }

    void output_end_indent() override { close_pop(); }

    void output_begin_scope(string name) override {
      auto scope_element=localize(element_t::make_root("scope"));
      scope_element
        ->append_attribute("name", escape_tilde(name));
      push_open(scope_element);
    }

    void output_end_scope(string) override { close_pop(); }

    void output_begin_declare_scope(string code_str) override {
      auto declare_scope_element=
        localize(element_t::make_root("declare_scope"));
      declare_scope_element
        ->append_attribute("declare", escape_tilde(code_str));
      push_open(declare_scope_element);
    }

    void output_end_declare_scope() override { close_pop(); }

    void output_separator() override {
      auto separator_element=
        localize(element_t::make_root("separator"));
      print(separator_element);
    }

    void output_step_id(string id) override {
      auto step_id_element=localize(element_t::make_root("step_id"));
      step_id_element
        ->append_attribute("id", escape_tilde(id));
      print(step_id_element);
    }

    void output_text(string text) override {
      auto text_element=localize(element_t::make_root("text"));
      text_element
        ->append_text(escape_tilde(text));
      print(text_element);
    }

    void output_declare(string code_str) override {
      auto declare_element=localize(element_t::make_root("declare"));
      declare_element
        ->append_text(escape_tilde(code_str));
      print(declare_element);
    }

    void output_perform(string code_str) override {
      auto perform_element=localize(element_t::make_root("perform"));
      perform_element
        ->append_text(escape_tilde(code_str));
      print(perform_element);
    }

    void output_try(string code_str, bool) override {
      auto try_element=localize(element_t::make_root("try"));
      try_element
        ->append_text(escape_tilde(code_str));
      print(try_element);
    }

    void output_catch(string exception_type, string error,
                      string caught, bool) override {
      auto catch_element=localize(element_t::make_root("catch"));
      catch_element
        ->append_text(escape_tilde(error))
        ->append_attribute("exception_type", escape_tilde(exception_type))
        ->append_attribute("success", xml_bool(caught));
      print(catch_element);
    }

    void output_show_value(string expr_str, string value_str) override {
      auto show_value_element=
        localize(element_t::make_root("show_value"));
      show_value_element
        ->add_element("expression1")
          ->append_text(escape_tilde(expr_str))
          ->append_attribute("value", escape_tilde(value_str));
      print(show_value_element);
    }

    void output_begin_with(string var,
                           string container_first, string container_rest,
                           string summary) override {
      auto with_element=localize(element_t::make_root("with"));
      with_element
        ->append_attribute("var", escape_tilde(var))
        ->append_attribute("container_first", escape_tilde(container_first))
        ->append_attribute("container_rest", escape_tilde(container_rest))
        ->append_attribute("summary", escape_tilde(summary));
      push_open(with_element);
    }

    void output_end_with() override { close_pop(); }

    void output_begin_with_results() override {
      auto with_results_element=element_t::make_root("with_results");
      push_open(with_results_element);
    }

    void output_end_with_results() override { close_pop(); }

    void output_general_summary(string element_tag, string name,
                                TestStats test_stats) {
      auto
        n_failed=test_stats.n_failed(),
        n_total=n_failed+test_stats.n_passed(),
        n_errors=test_stats.n_errors();
      auto general_summary_element=element_t::make_root(element_tag);
      general_summary_element
        ->append_attribute("name", name)
        ->append_attribute("n_failed", to_string(n_failed))
        ->append_attribute("n_total", to_string(n_total))
        ->append_attribute("n_errors", to_string(n_errors))
        ->append_attribute("success", xml_bool(n_failed+n_errors==0));
      print(general_summary_element);
    }

    void output_with_summary(string name, TestStats test_stats) override
      { output_general_summary("with_stats", name, test_stats); }

    auto add_check_element(string element_name, string success,
                           string prefix) {
      auto check_element=localize(element_t::make_root(element_name));
      check_element
        ->append_attribute("success", xml_bool(success))
        ->append_attribute("prefix", prefix);
      return check_element;
     }

    void output_check_true(string expr_str, string success,
                           string prefix, bool) override {
      auto check_element=add_check_element("check_true", success, prefix);
      check_element
        ->add_element("expression1")
          ->append_text(escape_tilde(expr_str));
      print(check_element);
    }

    void output_check_true_for(string expr_str,
                               string exprv_str, string valv_str,
                               string success,
                               string prefix, bool) override {
      auto check_element=add_check_element("check_true_for", success, prefix);
      check_element
        ->add_element("expression1")
          ->append_text(escape_tilde(expr_str));
      check_element
        ->add_element("expressionv")
        ->append_attribute("value", escape_tilde(valv_str))
        ->append_text(escape_tilde(exprv_str));
      print(check_element);
    }

    void output_check_equal(string expr1_str, string val1_str,
                            string expr2_str, string val2_str,
                            string success,
                            string prefix, bool) override {
      auto check_element=add_check_element("check_equal", success, prefix);
      check_element
        ->add_element("expression1")
          ->append_attribute("value", escape_tilde(val1_str))
          ->append_text(escape_tilde(expr1_str));
      check_element
        ->add_element("expression2")
          ->append_attribute("value", escape_tilde(val2_str))
          ->append_text(escape_tilde(expr2_str));
      print(check_element);
    }

    void output_check_approx(string expr1_str, string val1_str,
                             string expr2_str, string val2_str,
                             string max_error_str,
                             string success,
                             string prefix, bool) override {
      auto check_element=add_check_element("check_approx", success, prefix);
      check_element
        ->append_attribute("max_error", escape_tilde(max_error_str));
      check_element
        ->add_element("expression1")
          ->append_attribute("value", escape_tilde(val1_str))
          ->append_text(escape_tilde(expr1_str));
      check_element
        ->add_element("expression2")
          ->append_attribute("value", escape_tilde(val2_str))
          ->append_text(escape_tilde(expr2_str));
      print(check_element);
    }

    void produce_summary(string title, TestStats test_stats) override {
      output_general_summary("stats", title, test_stats);
      close_pop();
    }

    void uncaught_exception(string exception) override {
      auto uncaught_exception_element=
        element_t::make_root("uncaught_exception");
      uncaught_exception_element
        ->append_text(escape_tilde(exception));
      print(uncaught_exception_element);
    }

    void print_test_readout() const override { } // already wrote everything

  private:
    inline static pattern::register_creator<TestFormatXML>
      rc{test_format_named_creator(), "xml"};
  };

}
