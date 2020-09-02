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

#ifndef TESTIT_TESTFORMAT_HEADER_
#define TESTIT_TESTFORMAT_HEADER_

#include "testudo_stats.h"
#include "named_create.h"
#include <string>
#include <cassert>

namespace testudo {

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
    virtual ~TestFormat() { };
    virtual void output_title(std::string name, std::string title)=0;
    virtual void output_begin_scope(std::string name)=0;
    virtual void output_end_scope(std::string name)=0;
    virtual void output_separator()=0;
    virtual void output_step_id(std::string id)=0;
    virtual void output_text(std::string text)=0;
    virtual void output_multiline_text(std::string text)=0;
    virtual void output_declare(std::string code_str)=0;
    virtual void output_perform(std::string code_str)=0;
    virtual void output_try(std::string code_str)=0;
    virtual void output_catch(std::string exception_type, std::string error,
                              bool caught)=0;
    virtual void output_show_value(
      std::string expr_str, std::string value_str)=0;
    virtual void output_show_multiline_value(
      std::string expr_str, std::string value_str)=0;
    virtual void output_check_true(std::string expr_str, bool expr)=0;
    virtual void output_check_equal(
      std::string expr1_str, std::string val1_str,
      std::string expr2_str, std::string val2_str,
      bool equal)=0;
    virtual void output_check_approx(
      std::string expr1_str, std::string val1_str,
      std::string expr2_str, std::string val2_str,
      std::string max_error_str,
      bool approx)=0;
    virtual void output_check_verify(
      std::string expr_str, std::string val_str,
      std::string pred_str,
      bool verify)=0;
    virtual void uncaught_exception(std::string exception)=0;
    virtual void produce_summary(std::string name, TestStats test_stats)=0;

    virtual void print_test_readout() const=0;
  };

  using test_format_p=std::shared_ptr<TestFormat>;
  using test_format_pc=std::shared_ptr<TestFormat const>;

  define_named_creator(test_format_named_creator,
                       TestFormatNamedCreator,
                       TestFormat,
                       std::ostream &);

  // the following implementation is just meant for fixtures, when their
  // workings aren't shown (therefore, it mustn't be registered for creation by
  // name); methods that shouldn't be used in that situation are implemented as
  // "assert(false)"; some methods must delegate to the actual test format used
  // for the non-fixture part of the test; the other methods just do nothing
  class NullTestFormatForFixtures
    : public TestFormat {
  public:
    NullTestFormatForFixtures(test_format_p non_fixture_test_format)
      : non_fixture_test_format(non_fixture_test_format) { }
    void output_title(std::string, std::string) override { assert(false); }
    void output_begin_scope(std::string) override { }
    void output_end_scope(std::string) override { }
    void output_separator() override { }
    void output_step_id(std::string) override { }
    void output_text(std::string) override { }
    void output_multiline_text(std::string) override { }
    void output_declare(std::string) override { }
    void output_perform(std::string) override { }
    void output_try(std::string) override { }
    void output_catch(std::string, std::string, bool) override { }
    void output_show_value(std::string, std::string) override { }
    void output_show_multiline_value(std::string, std::string) override { }
    void output_check_true(std::string, bool) override { }
    void output_check_equal(std::string, std::string,
                            std::string, std::string,
                            bool) override { }
    void output_check_approx(std::string, std::string,
                            std::string, std::string,
                             std::string,
                             bool) override { }
    void output_check_verify(std::string, std::string,
                             std::string,
                             bool) override { }
    void uncaught_exception(std::string exception) override
      { non_fixture_test_format->uncaught_exception(exception); }
    void produce_summary(std::string, TestStats) override { assert(false); }
    void print_test_readout() const override { }
  private:
    test_format_p const non_fixture_test_format;
  };

}

#endif
