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
#include "crc.h"
#include <iostream>

namespace {

  using namespace testudo;
  using namespace cyclic_redundancy_check;
  using namespace std;

  // line categories:
  //
  //     * '[' location info
  //
  //     * 't-' title, summary
  //
  //     * 'c-' check
  //
  //     * 'r-' check result, "with" summary
  //
  //     * 'i-' informative: declarations, statements, scopes...
  //
  //     * 'v-' value display
  //
  //     * 'e-' error

  class TestFormatTrack
    : public TestFormat {
  public:
    ostream &ts;
    TestFormatTrack(ostream &os) : ts(os) { }

    string encode_location() { return  "["+get_location()+"] "; }
    string encode_stats(TestStats test_stats, bool informative=false) {
      return (informative ? "i-" : "r-")+to_string(test_stats.n_passed())
        +"-"+to_string(test_stats.n_failed())
        +"-"+to_string(test_stats.n_errors());
    }

    string crc(string code)
      { return "["+to_hex(crc64(code))+"]"; }
    string crc(string code, string content)
      { return crc(code+" "+content); }
    void encode(char category, string code)
      { ts << category << "-" << code << "\n"; }
    void encode(char category, string code, string content)
      { ts << category << "-" << code << " " << crc(code, content) << "\n"; }
    void encode_check(string code, string content,
                      string success, bool informative) {
      if (success=="with") {
        assert(informative);
        encode('i', code, content);
      }
      else {
        bool success_b=to_bool(success);
        ts << encode_location()
           << (informative ? 'i' : 'c') << "-"
           << code << " " << crc(code, content) << " "
           << encode_stats({success_b, not success_b, 0}, informative) << "\n";
      }
    }
    string check_try_code_str;
    void encode_check_try(string code_str) {
      assert(check_try_code_str.empty());
      check_try_code_str=code_str;
    }
    void encode_check_catch(string exception_type,
                            string success, bool informative) {
      encode_check("try-catch ", check_try_code_str+exception_type,
                   success, informative);
      check_try_code_str="";
    }
    void encode_summary(string code, string content, TestStats test_stats) {
      ts << encode_location()
         << "c-" << code << " " << crc(code, content) << " "
         << encode_stats(test_stats) << "\n";
    }
    void encode_uncaught(string exception) {
      ts << encode_location()
         << "e-uncaught_exception " << crc(exception) << " "
         << encode_stats({0, 0, 1}) << "\n";
    }

    void output_title(string name, string title) override
      { encode('t', "title", name+title); }
    void output_begin_indent() override
      { encode('i', "begin_indent"); }
    void output_end_indent() override
      { encode('i', "end_indent"); }
    void output_begin_scope(string name) override
      { encode('i', "begin_scope", name); }
    void output_end_scope(string name) override
      { encode('i', "end_scope", name); }
    void output_begin_declare_scope(string code_str) override
      { encode('i', "begin_declare_scope", code_str); }
    void output_end_declare_scope() override
      { encode('i', "end_declare_scope"); }
    void output_separator() override
      { encode('i', "separator"); }
    void output_step_id(string id) override
      { encode('i', "step_id", id); }
    void output_text(string text) override
      { encode('i', "text", text);  }
    void output_declare(string code_str) override
      { encode('i', "declare", code_str); }
    void output_perform(string code_str) override
      { encode('i', "perform", code_str); }
    void output_try(string code_str, bool) override
      { encode_check_try(code_str); }
    void output_catch(string exception_type, string,
                      string caught, bool informative) override
      { encode_check_catch(exception_type, caught, informative); }
    void output_show_value(
        string expr_str, string value_str) override
      { encode('v', "show_value", expr_str+value_str); }
    void output_begin_with(string var_name,
                           string container_first, string container_rest,
                           string) override
      { encode('i', "begin_with", var_name+container_first+container_rest); }
    void output_end_with() override
      { encode('i', "end_with"); }
    void output_begin_with_results() override
      { encode('i', "begin_with_results"); }
    void output_end_with_results() override
      { encode('i', "end_with_results"); }
    void output_with_summary(string name, TestStats test_stats) override
      { encode_summary("with_summary", name, test_stats); }
    void output_check_true(string expr_str, string success,
                           string prefix, bool informative) override
      { encode_check("check_true", prefix+expr_str, success, informative); }
    void output_check_true_for(string expr_str,
                               string exprv_str, string,
                               string success,
                               string prefix, bool informative) override {
      encode_check("check_true_for", prefix+expr_str+exprv_str,
                   success, informative);
    }
    void output_check_equal(string expr1_str, string,
                            string expr2_str, string,
                            string success,
                            string prefix, bool informative) override {
      encode_check("check_equal", prefix+expr1_str+expr2_str,
                   success, informative);
    }
    void output_check_approx(string expr1_str, string,
                             string expr2_str, string,
                             string max_error_str,
                             string success,
                             string prefix, bool informative) override {
        encode_check("check_approx",
                     prefix+expr1_str+expr2_str+max_error_str,
                     success, informative);
    }
    void uncaught_exception(string exception) override
      { encode_uncaught(exception); }
    //  we ignore the "TestStats" for "produce_summary()" since we want to
    // detect changes in the individual steps, not in the aggregated stats
    void produce_summary(string name, TestStats) override
      { encode('t', "produce_summary", name); }

    void print_test_readout() const override { } // already wrote everything

  private:
    inline static pattern::register_creator<TestFormatTrack>
      rc{test_format_named_creator(), "track"};
  };

}
