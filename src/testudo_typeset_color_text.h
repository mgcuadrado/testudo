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

#ifndef MGCUADRADO_TESTUDO_TYPESET_COLOR_TEXT_HEADER_
#define MGCUADRADO_TESTUDO_TYPESET_COLOR_TEXT_HEADER_

#include <iosfwd>
#include <string>
#include <list>
#include <memory>

namespace testudo___implementation {

  class TextTypeset {
  public:
    using string=std::string;

    virtual ~TextTypeset()=default;

    virtual void interactive_test()=0;

    virtual void title(string location, string name, string title)=0;

    virtual void location(string brief_location)=0;

    virtual void begin_indent()=0;
    virtual void end_indent()=0;

    virtual void begin_scope(string name)=0;
    virtual void end_scope(string name)=0;

    virtual void begin_declare_scope(string code_str)=0;
    virtual void end_declare_scope()=0;

    virtual void separator()=0;

    virtual void step_id(string id)=0;

    virtual void text(string text)=0;

    virtual void declare(string code_str)=0;

    virtual void perform(string code_str)=0;

    virtual void try_catch_try(string code_str)=0;
    virtual void try_catch_catch(string exception_type, string error,
                                 string caught)=0;

    virtual void show_value(string expr_str, string value_str)=0;

    virtual void begin_with(string var_name,
                            string container_first, string container_rest)=0;
    virtual void end_with()=0;

    virtual void begin_with_results()=0;
    virtual void end_with_results()=0;

    virtual void with_summary(string name,
                              string n_failed, string n_total, string n_errors,
                              string success)=0;

    virtual void summary(string name,
                         string n_failed, string n_total, string n_errors,
                         string success)=0;

    virtual void check_true(string expr_str,
                            string exprv_str, string valv_str,
                            string explanation,
                            string success,
                            string prefix)=0;
    virtual void check_equal(string expr1_str, string val1_str,
                             string expr2_str, string val2_str,
                             string exprv_str, string valv_str,
                             string explanation,
                             string success,
                             string prefix)=0;
    virtual void check_approx(string expr1_str, string val1_str,
                              string expr2_str, string val2_str,
                              string max_error_str,
                              string exprv_str, string valv_str,
                              string explanation,
                              string success,
                              string prefix)=0;

    virtual void uncaught_exception(string exception)=0;

    virtual void aborted(string message)=0;
  };

  class TextTrackTypeset {
  public:
    using string=std::string;

    virtual ~TextTrackTypeset()=default;

    virtual void no_changes()=0;
    virtual void track_header(string evolution, string count,
                              string stats_from, string stats_to,
                              string delta_stats)=0;
    virtual void track_entry(string location, string type,
                             string stats_from, string stats_to,
                             string delta_stats, bool show_label)=0;
  };

  std::string::size_type utf8_length(std::string const &s);

  unsigned const default_max_line_length=78;

  std::shared_ptr<TextTypeset>
  text_report_typeset(std::ostream &os,
                      unsigned max_line_length, bool show_location);
  std::shared_ptr<TextTypeset>
  color_text_report_typeset(std::ostream &os,
                            unsigned max_line_length, bool show_location);

  std::shared_ptr<TextTypeset>
  text_summary_typeset(std::ostream &os,
                       unsigned max_line_length);
  std::shared_ptr<TextTypeset>
  color_text_summary_typeset(std::ostream &os,
                             unsigned max_line_length);

  std::shared_ptr<TextTrackTypeset>
  text_track_typeset(std::ostream &os, unsigned max_line_length);
  std::shared_ptr<TextTrackTypeset>
  color_text_track_typeset(std::ostream &os, unsigned max_line_length);

}

#endif
