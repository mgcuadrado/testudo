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
#include "testudo_typeset_color_text.h"
#include <ostream>
#include <cassert>

namespace {

  using namespace testudo;
  using namespace std;
  using namespace testudo___implementation;

  class TestFormatColorText
    : public TestFormat {
  public:
    TestFormatColorText(ostream &os, bool color=true, bool show_location=true)
      : typeset((color ? color_text_report_typeset : text_report_typeset)(
                  os, default_max_line_length, show_location)) { }
  private:
    shared_ptr<TextTypeset> const typeset;
  public:
    void output_title(string name, string title) override {
      typeset->title(get_title_location(), name, title);
    }

    void output_location() {
      // it'd be great to be able to flag the "location.empty()" situation as
      // a Testudo error, but fttb i'm using it to avoid outputting location
      // info in cases where we know we don't have to; i should be more
      // precise here, so i should "assert(not get_location())", and enforce
      // the fact that in those situations we are explicitly /not/ outputting
      // location info
      typeset->location(get_brief_location());
      set_location({});
    }

    void output_begin_indent() override {
      typeset->begin_indent();
    }
    void output_end_indent() override {
      typeset->end_indent();
    }

    void output_begin_scope(string name) override {
      output_location();
      typeset->begin_scope(name);
    }
    void output_end_scope(string name) override {
      typeset->end_scope(name);
    }

    void output_begin_declare_scope(string code_str) override {
      output_location();
      typeset->begin_declare_scope(code_str);
    }
    void output_end_declare_scope() override {
      typeset->end_declare_scope();
    }

    void output_separator() override {
      output_location();
      typeset->separator();
    }

    void output_step_id(string id) override {
      output_location();
      typeset->step_id(id);
    }

    void output_text(string text) override {
      // don't print location for multiline text:
      if (text.find('\n')==string::npos)
        output_location();
      typeset->text(text);
    }

    void output_declare(string code_str) override {
      output_location();
      typeset->declare(code_str);
    }

    void output_perform(string code_str) override {
      output_location();
      typeset->perform(code_str);
    }

    void output_try(string code_str, bool) override {
      output_location();
      typeset->try_catch_try(code_str);
    }
    void output_catch(string exception_type, string error,
                      string caught, bool) override {
      typeset->try_catch_catch(exception_type, error, caught);
    }

    void output_show_value(string expr_str, string value_str) override {
      output_location();
      typeset->show_value(expr_str, value_str);
    }

    void output_begin_with(string var_name,
                           string container_first, string container_rest,
                           string) override {
      output_location();
      typeset->begin_with(var_name, container_first, container_rest);
    }
    void output_end_with() override { typeset->end_with(); }

    void output_begin_with_results() override
      { typeset->begin_with_results(); }
    void output_end_with_results() override
      { typeset->end_with_results(); }

    template <typename M>
    void output_summary(M summary, string name, TestStats test_stats) {
      auto
        n_failed=test_stats.n_failed(),
        n_total=n_failed+test_stats.n_passed(),
        n_errors=test_stats.n_errors();
      ((*typeset).*summary)(name,
                            to_string(n_failed),
                            to_string(n_total),
                            to_string(n_errors),
                            bool_to_string(n_failed+n_errors==0));
    }

    void output_with_summary(string name, TestStats test_stats) override
      { output_summary(&TextTypeset::with_summary, name, test_stats); }


    void output_check_true(string expr_str,
                           string exprv_str, string valv_str,
                           string explanation,
                           string success,
                           string prefix, bool) override {
      output_location();
      typeset->check_true(
        expr_str,
        exprv_str, valv_str, explanation,
        success, prefix);
    }

    void output_check_equal(string expr1_str, string val1_str,
                            string expr2_str, string val2_str,
                            string exprv_str, string valv_str,
                            string explanation,
                            string success,
                            string prefix, bool) override {
      output_location();
      typeset->check_equal(
        expr1_str, val1_str, expr2_str, val2_str,
        exprv_str, valv_str, explanation,
        success, prefix);
    }

    void output_check_approx(string expr1_str, string val1_str,
                             string expr2_str, string val2_str,
                             string max_error_str,
                             string exprv_str, string valv_str,
                             string explanation,
                             string success,
                             string prefix, bool) override {
      output_location();
      typeset->check_approx(
        expr1_str, val1_str, expr2_str, val2_str, max_error_str,
        exprv_str, valv_str, explanation,
        success, prefix);
    }

    void produce_summary(string name, TestStats test_stats) override
      { output_summary(&TextTypeset::summary, name, test_stats); }

    void uncaught_exception(string exception) override {
      typeset->uncaught_exception(exception);
    }

    void print_test_readout() const override { } // already wrote everything

  private:
    inline static pattern::register_creator<TestFormatColorText>
      rc{test_format_named_creator(), "color_text"};
  };

  class TestFormatText
    : public TestFormatColorText {
  public:
    TestFormatText(ostream &os, bool show_location=true)
      : TestFormatColorText(os, false, show_location) { }
  private:
    inline static pattern::register_creator<TestFormatText>
      rc{test_format_named_creator(), "text"};
  };

}
