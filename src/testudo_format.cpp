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
#include <sstream>

namespace testudo___implementation {

  using namespace std;

  pair<string, string>
  TestFormat::var_and_values_format(var_values_t var_values) {
    ostringstream oss_var, oss_value;
    if (var_values.size()==1) {
      oss_var << var_values.front().first;
      oss_value << var_values.front().second;
    }
    else {
      bool first_time=true;
      oss_var << "(";
      oss_value << "(";
      for (auto var_value: var_values) {
        oss_var << (first_time ? "" : ", ") << var_value.first;
        oss_value << (first_time ? "" : ", ") << var_value.second;
        first_time=false;
      }
      oss_var << ")";
      oss_value << ")";
    }
    return {oss_var.str(), oss_value.str()};
  }

  class NullTestFormatForFixtures
    : public TestFormat {
  public:
    NullTestFormatForFixtures(test_format_p parent_test_format)
      : parent_test_format(parent_test_format) { }

    void output_title(string, string) override { assert(false); }
    void output_begin_scope(string) override { }
    void output_end_scope(string) override { }
    void output_begin_declare_scope(string) override { }
    void output_end_declare_scope() override { }
    void output_separator() override { }
    void output_step_id(string) override { }
    void output_text(string) override { }
    void output_multiline_text(string) override { }
    void output_declare(string) override { }
    void output_perform(string) override { }
    void output_try(string) override { }
    void output_catch(string, string, string) override { }
    void output_show_value(string, string) override { }
    void output_show_multiline_value(string, string) override { }
    void output_begin_with(string, string) override { }
    void output_end_with() override { }
    void output_begin_with_results() override { }
    void output_end_with_results() override { }
    void output_with_summary(string, TestStats) override { }
    void output_check_true(string, string, string) override { }
    void output_check_equal(
      string, string, string, string, string, string) override { }
    void output_check_approx(
      string, string, string, string, string, string, string) override { }
    void output_check_verify(
      string, string, string, string, string) override { }

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
    void add(function<void ()> action)
      { actions.push_back(action); }
    void output() {
      if (not actions.empty()) {
        test_format->output_begin_with_results();
        for (auto const &action: actions)
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
    list<function<void ()>> actions;
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
                       string var_name, string val, string container)
      : parent_test_format(parent_test_format),
        non_with_ancestor(find_non_with_ancestor(parent_test_format)),
        log(log),
        recursively_last_time(
          last_time and parent_recursively_last_time(parent_test_format)),
        full_var_values(compute_full_var_values(var_name, val)),
        loop_name(var_name+" in "+container) {
      log->reset_counter();
      output_begin_with(var_name, container);
    }
    ~WithLoopTestFormat() {
      if (recursively_last_time
          and not dynamic_pointer_cast<WithLoopTestFormat>(parent_test_format))
        non_with_ancestor
          ->output_with_summary(loop_name, log->test_stats_diff());
      output_end_with();
    }

    void set_location(location_t location) {
      if (recursively_last_time)
        non_with_ancestor->set_location(location);
    }

    auto child_log() { return log->child(); }

    void failed() {
      log->failed();
      if (auto parent_with_loop=
            dynamic_pointer_cast<WithLoopTestFormat>(parent_test_format))
        parent_with_loop->failed();
    }
    bool if_false_output_with_failed(string success,
                                     function<void ()> error_output) {
      if (success=="false") {
        log->add([nwa=non_with_ancestor,
                  fvvf=var_and_values_format(full_var_values)]()
                   { nwa->output_show_value(fvvf.first, fvvf.second); });
        log->add(error_output);
        failed();
        return true;
      }
      else
        return false;
    }

    void output_title(string, string) override { assert(false); }
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
    void output_multiline_text(string text) override {
      if (recursively_last_time)
        non_with_ancestor->output_multiline_text(text);
    }
    void output_declare(string code_str) override {
      if (recursively_last_time)
        non_with_ancestor->output_declare(code_str);
    }
    void output_perform(string code_str) override {
      if (recursively_last_time)
        non_with_ancestor->output_perform(code_str);
    }
    void output_try(string code_str) override {
      if (recursively_last_time)
        non_with_ancestor->output_try(code_str);
    }
    void output_catch(string exception_type, string error,
                      string caught) override {
      log->incr_counter();
      if_false_output_with_failed(
        caught,
        [=, nwa=non_with_ancestor]()
          { nwa->output_catch(exception_type, error, caught); });
      if (recursively_last_time) {
        non_with_ancestor->output_catch(exception_type, "", "with");
        log->output();
      }
    }
    void output_show_value(string, string) override { }
    void output_show_multiline_value(string, string) override { }
    void output_begin_with(string var_name, string container) override {
      log->incr_counter();
      if (recursively_last_time)
        non_with_ancestor->output_begin_with(var_name, container);
    }
    void output_end_with() override {
      if (recursively_last_time)
        non_with_ancestor->output_end_with();
    }
    void output_begin_with_results() override { assert(false); }
    void output_end_with_results() override { assert(false); }
    void output_with_summary(string, TestStats) override { assert(false); }

    void output_check_true(string expr_str, string success,
                           string prefix) override {
      log->incr_counter();
      if_false_output_with_failed(
        success,
        [=, nwa=non_with_ancestor]() {
          nwa->output_check_true(expr_str, success, prefix);
        });
      if (recursively_last_time) {
        non_with_ancestor->output_check_true(expr_str, "with", prefix);
        log->output();
      }
    }
    void output_check_equal(string expr1_str, string val1_str,
                            string expr2_str, string val2_str,
                            string success,
                            string prefix) override {
      log->incr_counter();
      if_false_output_with_failed(
        success,
        [=, nwa=non_with_ancestor]() {
          nwa->output_check_equal(expr1_str, val1_str, expr2_str, val2_str,
                                  success, prefix);
        });
      if (recursively_last_time) {
        non_with_ancestor->output_check_equal(
          expr1_str, "", expr2_str, "", "with", prefix);
        log->output();
      }
    }
    void output_check_approx(string expr1_str, string val1_str,
                             string expr2_str, string val2_str,
                             string max_error_str,
                             string success,
                             string prefix) override {
      log->incr_counter();
      if_false_output_with_failed(
        success,
        [=, nwa=non_with_ancestor]() {
          nwa->output_check_approx(expr1_str, val1_str, expr2_str, val2_str,
                                   max_error_str, success, prefix);
        });
      if (recursively_last_time) {
        non_with_ancestor->output_check_approx(
          expr1_str, "", expr2_str, "", max_error_str, "with", prefix);
        log->output();
      }
    }
    void output_check_verify(string expr_str, string val_str,
                             string pred_str,
                             string success,
                             string prefix) override {
      log->incr_counter();
      if_false_output_with_failed(
        success,
        [=, nwa=non_with_ancestor]() {
          nwa->output_check_verify(expr_str, val_str, pred_str,
                                   success, prefix);
        });
      if (recursively_last_time) {
        non_with_ancestor->output_check_verify(
          expr_str, "", pred_str, "with", prefix);
        log->output();
      }
    }

    void uncaught_exception(string exception) override
      { parent_test_format->uncaught_exception(exception); }
    void produce_summary(string, TestStats) override { assert(false); }
    void print_test_readout() const override { assert(false); }
  private:
    test_format_p const parent_test_format, non_with_ancestor;
    shared_ptr<WithLoopLog> const log;
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
                             string var_name, string val, string container) {
    return make_shared<WithLoopTestFormat>(parent_test_format,
                                           log,
                                           last_time,
                                           var_name, val, container);
  }

}
