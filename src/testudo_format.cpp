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
#include <sstream>

namespace testudo___implementation {

  using namespace std;

  class NullTestFormatForFixtures
    : public TestFormat {
  public:
    NullTestFormatForFixtures(test_format_p parent_test_format)
      : parent_test_format(parent_test_format) { }

    void output_title(string, string) override { assert(false); }
    void output_begin_indent() override { }
    void output_end_indent() override { }
    void output_begin_scope(string) override { }
    void output_end_scope(string) override { }
    void output_begin_declare_scope(string) override { }
    void output_end_declare_scope() override { }
    void output_separator() override { }
    void output_step_id(string) override { }
    void output_text(string) override { }
    void output_declare(string) override { }
    void output_perform(string) override { }
    void output_try(string, bool) override { }
    void output_catch(string, string, string, bool) override { }
    void output_show_value(string, string) override { }
    void output_begin_with(string, string, string, string) override { }
    void output_end_with() override { }
    void output_begin_with_results() override { }
    void output_end_with_results() override { }
    void output_with_summary(string, TestStats) override { }
    void output_check_true(string, string, string, string,
                           string, string, bool) override { }
    void output_check_equal(
      string, string, string, string, string, string, string,
      string, string, bool) override { }
    void output_check_approx(
      string, string, string, string, string, string, string, string,
      string, string, bool) override { }

    void uncaught_exception(string exception) override
      { parent_test_format->uncaught_exception(exception); }
    void produce_summary(string, TestStats) override { assert(false); }
    void print_test_readout() const override { }
  private:
    test_format_p const parent_test_format;
  };

  shared_ptr<TestFormat>
  make_null_test_format_for_fixtures(test_format_p parent_test_format)
    { return make_shared<NullTestFormatForFixtures>(parent_test_format); }


  class WithLoopLog {
  public:
    WithLoopLog(test_format_p test_format, TestStats const &test_stats)
      : test_format(test_format), test_stats(test_stats),
        init_test_stats(test_stats) { }
    void reset_counter() { counter=0; }
    void incr_counter() {
      ++counter;
      if (counter>children.size()) {
        children.push_back(make_shared<WithLoopLog>(test_format, test_stats));
        assert(counter==children.size());
      }
    }
    auto child() { return children[counter-1]; }
    void add(string location, function<void ()> action)
      { actions[location].push_back(action); }
    void output(string location) {
      if (auto a_i=actions.find(location); a_i not_eq actions.end()) {
        test_format->output_begin_with_results();
        for (auto const &action: a_i->second)
          action();
        test_format->output_end_with_results();
      }
    }
    void failed() { all_successful_p=false; }
    bool all_successful() const { return all_successful_p; }
    TestStats test_stats_diff() const { return test_stats-init_test_stats; }
  private:
    test_format_p const test_format;
    TestStats const &test_stats;
    TestStats const init_test_stats;
    size_t counter;
    bool all_successful_p=true;
    vector<shared_ptr<WithLoopLog>> children;
    map<string, list<function<void ()>>> actions;
  };

  class WithLoopTestFormat
    : public TestFormat {
    static bool parent_recursively_last_time(test_format_p parent) {
      if (auto parent_with_loop=
            dynamic_pointer_cast<WithLoopTestFormat>(parent))
        return parent_with_loop.get()->recursively_last_time;
      else
        return true;
    }

    static test_format_p find_non_with_ancestor(test_format_p parent) {
      if (auto parent_with_loop=
            dynamic_pointer_cast<WithLoopTestFormat>(parent))
        return parent_with_loop->non_with_ancestor;
      else
        return parent;
    }

    var_values_t compute_full_var_values(string var_name, string val) const {
      var_values_t result;
      if (auto parent_with_loop=
            dynamic_pointer_cast<WithLoopTestFormat>(parent_test_format))
        result=parent_with_loop->full_var_values;
      result.push_back({var_name, val});
      return result;
    }

  public:
    WithLoopTestFormat(test_format_p parent_test_format,
                       shared_ptr<WithLoopLog> log, bool last_time,
                       string var_name, string val,
                       MultilineData container)
      : parent_test_format(parent_test_format),
        non_with_ancestor(find_non_with_ancestor(parent_test_format)),
        log(log),
        with_location(non_with_ancestor->get_full_location()),
        recursively_last_time(
          last_time and parent_recursively_last_time(parent_test_format)),
        full_var_values(compute_full_var_values(var_name, val)),
        loop_name(var_name+" in "+container.summary) {
      log->reset_counter();
      output_begin_with(var_name,
                        container.first_line, container.rest_lines,
                        loop_name);
    }
    ~WithLoopTestFormat() {
      if (recursively_last_time
          and not dynamic_pointer_cast<WithLoopTestFormat>(
                    parent_test_format)) {
        // reset the location of the opening "with_data()"
        non_with_ancestor->set_location(with_location);
        non_with_ancestor
          ->output_with_summary(loop_name, log->test_stats_diff());
      }
      output_end_with();
    }

    void set_location(location_t location) {
      TestFormat::set_location(location);
      if (recursively_last_time)
        non_with_ancestor->set_location(location);
    }

    auto child_log() { return log->child(); }

    void log_output(function<void ()> output)
      { log->add(get_location(), output); }
    void failed() {
      log->failed();
      if (auto parent_with_loop=
            dynamic_pointer_cast<WithLoopTestFormat>(parent_test_format))
        parent_with_loop->failed();
    }
    bool if_false_log_output_with_failed(string success,
                                         function<void ()> error_output) {
      if (success==false_tag) {
        log_output(error_output);
        failed();
        return true;
      }
      else
        return false;
    }

    void output_title(string, string) override { assert(false); }
    void output_begin_indent() override {
      if (recursively_last_time)
        non_with_ancestor->output_begin_indent();
    }
    void output_end_indent() override {
      if (recursively_last_time)
        non_with_ancestor->output_end_indent();
    }
    void output_begin_scope(string name) override {
      if (recursively_last_time)
        non_with_ancestor->output_begin_scope(name);
    }
    void output_end_scope(string name) override {
      if (recursively_last_time)
        non_with_ancestor->output_end_scope(name);
    }
    void output_begin_declare_scope(string code_str) override {
      if (recursively_last_time)
        non_with_ancestor->output_begin_declare_scope(code_str);
    }
    void output_end_declare_scope() override {
      if (recursively_last_time)
        non_with_ancestor->output_end_declare_scope();
    }
    void output_separator() override {
      if (recursively_last_time)
        non_with_ancestor->output_separator();
    }
    void output_step_id(string id) override {
      if (recursively_last_time)
        non_with_ancestor->output_step_id(id);
    }
    void output_text(string text) override {
      if (recursively_last_time)
        non_with_ancestor->output_text(text);
    }
    void output_declare(string code_str) override {
      if (recursively_last_time)
        non_with_ancestor->output_declare(code_str);
    }
    void output_perform(string code_str) override {
      if (recursively_last_time)
        non_with_ancestor->output_perform(code_str);
    }
    void output_try(string code_str, bool) override {
      if (recursively_last_time)
        non_with_ancestor->output_try(code_str, true);
    }
    void output_catch(string exception_type, string error,
                      string caught, bool) override {
      log->incr_counter();
      if_false_log_output_with_failed(
        caught,
        [=, nwa=non_with_ancestor]()
          { nwa->output_catch(exception_type, error, caught, false); });
      if (recursively_last_time) {
        non_with_ancestor->output_catch(exception_type, "", "with", true);
        log->output(get_location());
      }
    }
    void output_show_value(string expr_str, string value_str) override {
      log->incr_counter();
      log_output(
        [=, nwa=non_with_ancestor]() {
          nwa->output_show_value(expr_str, value_str);
        });
      if (recursively_last_time) {
        non_with_ancestor->output_show_value(expr_str, "");
        log->output(get_location());
      }
    }
    void output_begin_with(string var_name, string
                           container_first, string container_rest,
                           string summary) override {
      log->incr_counter();
      if (recursively_last_time)
        non_with_ancestor->output_begin_with(var_name,
                                             container_first, container_rest,
                                             summary);
    }
    void output_end_with() override {
      if (recursively_last_time)
        non_with_ancestor->output_end_with();
    }
    void output_begin_with_results() override { assert(false); }
    void output_end_with_results() override { assert(false); }
    void output_with_summary(string, TestStats) override { assert(false); }

    void output_check_true(string expr_str,
                           string exprv_str, string valv_str,
                           string explanation,
                           string success,
                           string prefix, bool) override {
      log->incr_counter();
      if_false_log_output_with_failed(
        success,
        [=, nwa=non_with_ancestor]() {
          nwa->output_check_true(
            expr_str,
            exprv_str, valv_str, explanation,
            success, prefix, false);
        });
      if (recursively_last_time) {
        non_with_ancestor->output_check_true(
          expr_str,
          exprv_str, "", "",
          "with", prefix, true);
        log->output(get_location());
      }
    }
    void output_check_equal(string expr1_str, string val1_str,
                            string expr2_str, string val2_str,
                            string exprv_str, string valv_str,
                            string explanation,
                            string success,
                            string prefix, bool) override {
      log->incr_counter();
      if_false_log_output_with_failed(
        success,
        [=, nwa=non_with_ancestor]() {
          nwa->output_check_equal(
            expr1_str, val1_str, expr2_str, val2_str,
            exprv_str, valv_str, explanation,
            success, prefix, false);
        });
      if (recursively_last_time) {
        non_with_ancestor->output_check_equal(
          expr1_str, "", expr2_str, "",
          exprv_str, "", "",
          "with", prefix, true);
        log->output(get_location());
      }
    }
    void output_check_approx(string expr1_str, string val1_str,
                             string expr2_str, string val2_str,
                             string max_error_str,
                             string exprv_str, string valv_str,
                             string explanation,
                             string success,
                             string prefix, bool) override {
      log->incr_counter();
      if_false_log_output_with_failed(
        success,
        [=, nwa=non_with_ancestor]() {
          nwa->output_check_approx(
            expr1_str, val1_str, expr2_str, val2_str, max_error_str,
            exprv_str, valv_str, explanation,
            success, prefix, false);
        });
      if (recursively_last_time) {
        non_with_ancestor->output_check_approx(
          expr1_str, "", expr2_str, "", max_error_str,
          exprv_str, "", "",
          "with", prefix, true);
        log->output(get_location());
      }
    }

    void uncaught_exception(string exception) override
      { parent_test_format->uncaught_exception(exception); }
    void produce_summary(string, TestStats) override { assert(false); }
    void print_test_readout() const override { assert(false); }
  private:
    test_format_p const parent_test_format, non_with_ancestor;
    shared_ptr<WithLoopLog> const log;
    location_t const with_location;
    bool const recursively_last_time;
    var_values_t const full_var_values;
    string const loop_name;
  };

  shared_ptr<WithLoopLog> get_with_loop_log(test_management_t parent) {
    if (auto parent_with_loop=
        dynamic_pointer_cast<WithLoopTestFormat>(parent.format))
      return parent_with_loop->child_log();
    else
      return make_shared<WithLoopLog>(parent.format, parent.stats);
  }

  shared_ptr<TestFormat>
  make_with_loop_test_format(test_format_p parent_test_format,
                             shared_ptr<WithLoopLog> log, bool last_time,
                             string var_name, string val,
                             MultilineData container) {
    return make_shared<WithLoopTestFormat>(parent_test_format,
                                           log,
                                           last_time,
                                           var_name, val, container);
  }

  MultilineData break_multiline_data(string d) {
    size_t i=d.find('{');
    if (i==string::npos)
      return {d, "", d};
    ++i;
    size_t level=0;
    string first_line=d.substr(0, i);
    string rest_lines;
    auto skip_spaces=
      [&d, &i]() {
        while ((i<d.size()) and ((d[i]==' ') or (d[i]=='\t') or (d[i]=='\n')))
          ++i;
      };
    skip_spaces();
    auto skip_until=
      [&d, &rest_lines, &i](char e) {
        rest_lines+=d[i++];
        while ((i<d.size()) and (d[i] not_eq e)) {
          if ((d[i]=='\\') and (i+1<d.size()))
            rest_lines+=d[i++];
          else if (d[i]==e)
            return;
          rest_lines+=d[i++];
        }
      };
    while (i<d.size()) {
      bool comma=false;
      switch (d[i]) {
      case '{': ++level; break;
      case '}': --level; break;
      case ',': comma=(level==0); break;
      case '\'': skip_until('\''); break;
      case '"': skip_until('"'); break;
      default: break;
      }
      rest_lines+=d[i++];
      if (comma) {
        rest_lines+="\n";
        skip_spaces();
      }
    }
    return {first_line, rest_lines, first_line+"...}"};
  }

  MultilineData break_data(string d) {
    size_t i=d.find('{');
    if (i==string::npos)
      return {d, "", d};
    return {d, "", d.substr(0, i+1)+"..."+d.substr(d.rfind('}'))};
  }

}
