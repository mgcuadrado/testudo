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

namespace testudo {

  using namespace std;

  namespace {

    using namespace kmsxml;

    string xml_bool(bool b) { return b ? "true" : "false"; }

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
          ->append_attribute("name", name)
          ->append_attribute("title", title);
      }

      void output_begin_scope(string name) {
        auto new_scope=test_stack.top()->add_element("scope");
        test_stack.push(new_scope);
        new_scope->append_attribute("name", name);
      }

      void output_end_scope(string)
        { test_stack.pop(); }

      void output_separator() override
        { test_stack.top()->add_element("separator"); }

      void output_step_id(string id) override {
        test_stack.top()
          ->add_element("step_id")
            ->append_attribute("id", id);
      }

      void output_text(string text) override
        { test_stack.top()->add_element("text")->append_text(text); }

      void output_multiline_text(string text) override
        { test_stack.top()->add_element("multiline_text")->append_text(text); }

      void output_declare(string code_str) override
        { test_stack.top()->add_element("declare")->append_text(code_str); }

      void output_perform(string code_str) override
        { test_stack.top()->add_element("perform")->append_text(code_str); }

      void output_try(string code_str) override {
        test_stack.top()
          ->add_element("try")
            ->append_text(code_str);
      }

      void output_catch(string exception_type, string error,
                        bool caught) override {
        test_stack.top()
          ->add_element("catch")
            ->append_text(error)
            ->append_attribute("exception_type", exception_type)
            ->append_attribute("success", xml_bool(caught));
      }

      void output_show_value(string expr_str, string value_str) override {
        auto show_value=test_stack.top()->add_element("show_value");
        show_value
          ->add_element("expression1")
            ->append_text(expr_str)
            ->append_attribute("value", value_str);
      }

      void output_show_multiline_value(
          string expr_str, string value_str) override {
        auto show_value=test_stack.top()->add_element("show_multiline_value");
        show_value
          ->add_element("expression1")
            ->append_text(expr_str)
            ->append_attribute("value", value_str);
      }

      void output_check_true(string expr_str, bool expr) override {
        test_stack.top()
          ->add_element("check_true")
            ->append_attribute("success", xml_bool(expr))
            ->add_element("expression1")
              ->append_text(expr_str);
      }

      void output_check_equal(string expr1_str, string val1_str,
                              string expr2_str, string val2_str,
                              bool equal) override {
        auto check_equal=test_stack.top()->add_element("check_equal");
        check_equal->append_attribute("success", xml_bool(equal));
        check_equal
          ->add_element("expression1")
            ->append_attribute("value", val1_str)
            ->append_text(expr1_str);
        check_equal
          ->add_element("expression2")
            ->append_attribute("value", val2_str)
            ->append_text(expr2_str);
      }

      void output_check_approx(string expr1_str, string val1_str,
                               string expr2_str, string val2_str,
                               string max_error_str,
                               bool approx) override {
        auto check_approx=test_stack.top()->add_element("check_approx");
        check_approx
          ->append_attribute("max_error", max_error_str)
          ->append_attribute("success", xml_bool(approx));
        check_approx
          ->add_element("expression1")
            ->append_attribute("value", val1_str)
            ->append_text(expr1_str);
        check_approx
          ->add_element("expression2")
            ->append_attribute("value", val2_str)
            ->append_text(expr2_str);
      }

      void output_check_verify(std::string expr_str, std::string val_str,
                               std::string pred_str,
                               bool verify) override {
        auto check_verify=test_stack.top()->add_element("check_verify");
        check_verify
          ->append_attribute("success", xml_bool(verify));
        check_verify
          ->add_element("expression1")
            ->append_attribute("value", val_str)
            ->append_text(expr_str);
        check_verify
          ->add_element("predicate")
            ->append_text(pred_str);
      }

      void produce_summary(string, TestStats test_stats) override {
        auto
          n_failed=test_stats.n_failed,
          n_total=n_failed+test_stats.n_passed,
          n_errors=test_stats.n_errors;
        test_stack.top()
          ->append_attribute("n_failed", to_string(n_failed))
          ->append_attribute("n_total", to_string(n_total))
          ->append_attribute("n_errors", to_string(n_errors))
          ->append_attribute("success",
                             xml_bool((n_failed==0) and (n_errors==0)));
        test_stack.pop();
      }

      void uncaught_exception(string exception) override {
        test_stack.top()
          ->add_element("uncaught_exception")
            ->append_text(exception);
      }

      void print_test_readout() const { ts << *root; }

    private:
      inline static pattern::register_creator<TestFormatXML>
        rc{test_format_named_creator(), "xml"};
    };

  }

}
