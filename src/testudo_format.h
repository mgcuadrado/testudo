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

#ifndef MGCUADRADO_TESTUDO_TESTFORMAT_HEADER_
#define MGCUADRADO_TESTUDO_TESTFORMAT_HEADER_

#include "testudo_macros.h"
#include "testudo_stats.h"
#include "named_create.h"
#include <string>
#include <list>
#include <cassert>

namespace testudo___implementation {

  // a test format is used by a running test to record the actions and checks
  // it performs; this abstract class declares the interface; each method
  // corresponds to something to record; the test driver will call each method
  // as actions and checks are performed, and "print_test_readout()" at the end
  // of the test, to give the test format a chance to dump everything; a
  // concrete implementation overloads these methods to write the related
  // information in a specific format; a test format implementation can choose
  // to write the information immediatly during the method invocation, or to
  // keep track of it internally and dump everything at the end of the test on
  // "print_test_readout()"
  //
  // test format concrete implementations must register themselves with
  // "test_format_named_creator()" (see "named_create" module), so they can be
  // selected by name
  class TestFormat {
  public:
    using string=std::string;
    struct location_t {
      inline static string common_directory="";
      string file, line;
      string to_string() const {
        string sfile=short_file();
        return (sfile.empty() ? "" : sfile+":")+line;
      }
      string to_string_brief(location_t title_location) const {
        return
          location_t{(file==title_location.file ? "" : file), line}
          .to_string();
      }
      string short_file() const {
        if ((not common_directory.empty())
            and (file.substr(0, common_directory.length()+1)
                 ==common_directory+"/"))
          return ".../"+file.substr(common_directory.length()+1);
        else
          return file;
      }
    };
    using var_values_t=std::list<std::pair<string, string>>;

    virtual ~TestFormat() { };

    virtual void set_location(location_t location) { location_p=location; }
    location_t get_full_location() const { return location_p; }
    virtual string get_location() const
      { return location_p.to_string(); }
    virtual string get_title_location() const
      { return title_location_p.to_string(); }
    virtual void set_title_location(location_t location)
      { title_location_p=location; }
    virtual string get_brief_location() const
      { return location_p.to_string_brief(title_location_p); }

    virtual void output_title(string name, string title)=0;
    virtual void output_begin_scope(string name)=0;
    virtual void output_end_scope(string name)=0;
    virtual void output_begin_declare_scope(string code_str)=0;
    virtual void output_end_declare_scope()=0;
    virtual void output_separator()=0;
    virtual void output_step_id(string id)=0;
    virtual void output_text(string text)=0;
    virtual void output_declare(string code_str)=0;
    virtual void output_perform(string code_str)=0;
    virtual void output_try(string code_str, bool informative)=0;
    virtual void output_catch(string exception_type, string error,
                              string caught, bool informative)=0;
    virtual void output_show_value(
      string expr_str, string value_str)=0;
    virtual void output_begin_with(string var_name,
                                   string container_first,
                                   string container_rest,
                                   string summary)=0;
    virtual void output_end_with()=0;
    virtual void output_begin_with_results()=0;
    virtual void output_end_with_results()=0;
    virtual void output_with_summary(string name, TestStats test_stats)=0;
    // in the following methods, "informative" means that the check has already
    // been tallied by a "with-data" structure, and it's being shown only to
    // show the actual values that caused the result; in most cases, this won't
    // make a difference in the resulting report, but e.g. in the "track"
    // format, this is used to mark the check as information rather than
    // trackable check result, both when listing the checks performed and when
    // reporting their results
    virtual void output_check_true(string expr_str, string success,
                                   string prefix,
                                   bool informative)=0;
    virtual void output_check_true_for(string expr_str,
                                       string exprv_str, string valv_str,
                                       string success,
                                       string prefix,
                                       bool informative)=0;
    virtual void output_check_equal(string expr1_str, string val1_str,
                                    string expr2_str, string val2_str,
                                    string success,
                                    string prefix,
                                    bool informative)=0;
    virtual void output_check_approx(string expr1_str, string val1_str,
                                     string expr2_str, string val2_str,
                                     string max_error_str,
                                     string success,
                                     string prefix,
                                     bool informative)=0;
    virtual void output_check_verify(string expr_str, string val_str,
                                     string pred_str,
                                     string success,
                                     string prefix,
                                     bool informative)=0;
    virtual void uncaught_exception(string exception)=0;
    virtual void produce_summary(string name, TestStats test_stats)=0;

    virtual void print_test_readout() const=0;

    static bool to_bool(string s) {
      if (s=="true")
        return true;
      else if (s=="false")
        return false;
      else {
        assert(false);
        return {};
      }
    }
    static std::string bool_to_string(bool b) { return b ? "true" : "false"; }

  protected:
    location_t title_location_p, location_p;
    std::pair<string, string> var_and_values_format(var_values_t);
  };

  using test_format_p=std::shared_ptr<TestFormat>;
  using test_format_pc=std::shared_ptr<TestFormat const>;

  struct test_management_t {
    test_format_p format;
    TestStats &stats;
  };

  define_named_creator(test_format_named_creator,
                       TestFormatNamedCreator,
                       TestFormat,
                       std::ostream &);

  // the following implementation is just meant for fixtures, when their
  // workings aren't shown (therefore, it mustn't be registered for creation by
  // name); methods that shouldn't be used in that situation are implemented as
  // "assert(false)"; some methods must delegate to the actual test format used
  // for the non-fixture part of the test; the other methods just do nothing
  std::shared_ptr<TestFormat>
  make_null_test_format_for_fixtures(test_format_p parent_test_format);

  struct MultilineData { std::string first_line, rest_lines, summary; };
  MultilineData break_multiline_data(std::string d);
  MultilineData break_data(std::string d);

  class WithLoopLog;
  std::shared_ptr<WithLoopLog> get_with_loop_log(test_management_t parent);
  // the following implementation is used for "with()"-loops; it calls
  // "output_begin_with()" and "output_end_with()", manages the first iteration
  // (the instructions must be reported on the first iteration only, so that we
  // have the contents of the "with()"-loop in the report once), handles
  // "output_begin_with()" and "output_end_with()" (on the first-first
  // iteration, "output_begin_with()" is passed down; on the last-last
  // iteration, "output_end_with()" is passed down), and reports a failed
  // iteration on destruction
  std::shared_ptr<TestFormat>
  make_with_loop_test_format(test_format_p parent_test_format,
                             std::shared_ptr<WithLoopLog> log, bool last_time,
                             std::string var_name, std::string val,
                             MultilineData container);

}

testudo___BRING(test_management_t,
                TestFormat,
                test_format_named_creator)

#endif
