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

#include "testudo_format.h"
#include "kmsxml.h"
#include <ostream>
#include <regex>

namespace {

  using namespace testudo;
  using namespace std;

  namespace {

    using namespace kmsxml;

    // replace all "~" instances with "||~tilde~||~"; codes in "||~"..."~||"
    // are assumed to have no length, so we add "~" after "||~tilde~||" so that
    // it contributes to the computed length as what it's going to be; we'll
    // have to simply remove all "||~tilde~||" instances at the end
    string escape_tilde(string s)
      { return regex_replace(s, regex("~"), "||~tilde~||~"); }

    string xml_bool(bool b) { return b ? "true" : "false"; }
    string xml_bool(string b) { return b; }

    class TestFormatXML
      : public TestFormat {
    public:
      element_t::root_t root{element_t::make_root("testudo")};
      stack<element_t::node_t<>> test_stack;
      ostream &ts;
      TestFormatXML(ostream &os) : ts(os) { test_stack.push(root); }

      void output_title(string name, string title) override {
        auto new_test=test_stack.top()->add_element("test");
        test_stack.push(new_test);
        new_test
          ->append_attribute("name", escape_tilde(name))
          ->append_attribute("title", escape_tilde(title));
      }

      void output_begin_scope(string name) {
        auto new_scope=test_stack.top()->add_element("scope");
        test_stack.push(new_scope);
        new_scope->append_attribute("name", escape_tilde(name));
      }

      void output_end_scope(string) { test_stack.pop(); }

      void output_begin_declare_scope(string code_str) {
        auto new_scope=test_stack.top()->add_element("declare_scope");
        test_stack.push(new_scope);
        new_scope->append_attribute("declare", escape_tilde(code_str));
      }

      void output_end_declare_scope() { test_stack.pop(); }

      void output_separator() override
        { test_stack.top()->add_element("separator"); }

      void output_step_id(string id) override {
        test_stack.top()
          ->add_element("step_id")
            ->append_attribute("id", escape_tilde(id));
      }

      void output_text(string text) override {
        test_stack.top()
          ->add_element("text")->append_text(escape_tilde(text));
      }

      void output_multiline_text(string text) override {
        test_stack.top()
          ->add_element("multiline_text")->append_text(escape_tilde(text));
      }

      void output_declare(string code_str) override {
        test_stack.top()
          ->add_element("declare")->append_text(escape_tilde(code_str));
      }

      void output_perform(string code_str) override {
        test_stack.top()
          ->add_element("perform")->append_text(escape_tilde(code_str));
      }

      void output_try(string code_str) override {
        test_stack.top()
          ->add_element("try")
            ->append_text(escape_tilde(code_str));
      }

      void output_catch(string exception_type, string error,
                        string caught) override {
        test_stack.top()
          ->add_element("catch")
            ->append_text(escape_tilde(error))
            ->append_attribute("exception_type", escape_tilde(exception_type))
            ->append_attribute("success", xml_bool(caught));
      }

      void output_show_value(string expr_str, string value_str) override {
        auto show_value=test_stack.top()->add_element("show_value");
        show_value
          ->add_element("expression1")
            ->append_text(escape_tilde(expr_str))
            ->append_attribute("value", escape_tilde(value_str));
      }

      void output_show_multiline_value(
          string expr_str, string value_str) override {
        auto show_value=test_stack.top()->add_element("show_multiline_value");
        show_value
          ->add_element("expression1")
            ->append_text(escape_tilde(expr_str))
            ->append_attribute("value", escape_tilde(value_str));
      }

      void output_begin_with(string var, string container) override {
        auto new_with_scope=test_stack.top()->add_element("with");
        test_stack.push(new_with_scope);
        new_with_scope
          ->append_attribute("var", var)
          ->append_attribute("container", container);
      }

      void output_end_with() override { test_stack.pop(); }

      void output_begin_with_results() override
        { test_stack.push(test_stack.top()->add_element("with_results")); }

      void output_end_with_results() override { test_stack.pop(); }

      void output_summary(TestStats test_stats) {
        auto
          n_failed=test_stats.n_failed(),
          n_total=n_failed+test_stats.n_passed(),
          n_errors=test_stats.n_errors();
        test_stack.top()
          ->append_attribute("n_failed", to_string(n_failed))
          ->append_attribute("n_total", to_string(n_total))
          ->append_attribute("n_errors", to_string(n_errors))
          ->append_attribute("success",
                             xml_bool((n_failed==0) and (n_errors==0)));
      }

      void output_with_summary(string, TestStats test_stats) override
        { output_summary(test_stats); }

      auto add_check_element(string element_name, string success,
                             string prefix) {
        auto check_element=test_stack.top()->add_element(element_name);
        return
          check_element
            ->append_attribute("success", xml_bool(success))
            ->append_attribute("prefix", prefix);
       }

      void output_check_true(string expr_str, string success,
                             string prefix) override {
        auto check_element=add_check_element("check_true", success, prefix);
        check_element
          ->add_element("expression1")
            ->append_text(escape_tilde(expr_str));
      }

      void output_check_equal(string expr1_str, string val1_str,
                              string expr2_str, string val2_str,
                              string success,
                              string prefix) override {
        auto check_element=add_check_element("check_equal", success, prefix);
        check_element
          ->add_element("expression1")
            ->append_attribute("value", escape_tilde(val1_str))
            ->append_text(escape_tilde(expr1_str));
        check_element
          ->add_element("expression2")
            ->append_attribute("value", escape_tilde(val2_str))
            ->append_text(escape_tilde(expr2_str));
      }

      void output_check_approx(string expr1_str, string val1_str,
                               string expr2_str, string val2_str,
                               string max_error_str,
                               string success,
                               string prefix) override {
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
      }

      void output_check_verify(string expr_str, string val_str,
                               string pred_str,
                               string success,
                               string prefix) override {
        auto check_element=add_check_element("check_verify", success, prefix);
        check_element
          ->add_element("expression1")
            ->append_attribute("value", escape_tilde(val_str))
            ->append_text(escape_tilde(expr_str));
        check_element
          ->add_element("predicate")
            ->append_text(escape_tilde(pred_str));
      }

      void produce_summary(string, TestStats test_stats) override {
        output_summary(test_stats);
        test_stack.pop();
      }

      void uncaught_exception(string exception) override {
        test_stack.top()
          ->add_element("uncaught_exception")
            ->append_text(escape_tilde(exception));
      }

      void print_test_readout() const { ts << *root; }

    private:
      inline static pattern::register_creator<TestFormatXML>
        rc{test_format_named_creator(), "xml"};
    };

  }

}
