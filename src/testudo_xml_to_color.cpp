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

#include "testudo_xml_to_color.h"
#include "testudo_format_xml.h" // for "unescape_tilde()"
#include "kmsxml.h"
#include "testudo_typeset_color_text.h"

using namespace std;
using namespace kmsxml;

namespace testudo___implementation {

  namespace {

    text_t attribute(element_t::node_const_t e, name_t name)
      { return unescape_tilde(get_attribute(e, name)); }

    text_t text(element_t::node_const_t e)
      { return unescape_tilde(get_text(e)); }

  }

  void testudo_xml_to_color(opts_t opts) {
    bool bw=false;
    bool summary=false;
    unsigned max_line_length=default_max_line_length;
    while (opts) {
      if (opts.opt("-b"))
        bw=true;
      else if (opts.opt("-s"))
        summary=true;
      else if (auto w=opts.opt_arg("-w"))
        max_line_length=stoi(w);
      else
        throw runtime_error("unknown option \""+opts.arg()+"\"");
    }
    auto typeset=
      summary
      ? (bw ? text_summary_typeset : color_text_summary_typeset)(
          cout, max_line_length)
      : (bw ? text_report_typeset : color_text_report_typeset)(
          cout, max_line_length, true);

    using enc_t=element_t::node_const_t;

    action_t process;

    action_t process_test=
      [&](enc_t e) {
        typeset->title(attribute(e, "location"),
                       attribute(e, "name"),
                       attribute(e, "title"));
        process(e);
        typeset->summary(attribute(e, "name"),
                         attribute(e, "n_failed"),
                         attribute(e, "n_total"),
                         attribute(e, "n_errors"),
                         attribute(e, "success"));
      };
    action_t process_scope=
      [&](enc_t e) {
        typeset->begin_scope(attribute(e, "name"));
        process(e);
        typeset->end_scope(attribute(e, "name"));
      };
    action_t process_declare_scope=
      [&](enc_t e) {
        typeset->begin_declare_scope(attribute(e, "declare"));
        process(e);
        typeset->end_declare_scope();
      };
    action_t process_separator=[&](enc_t) { typeset->separator(); };
    action_t process_step_id=
      [&](enc_t e) { typeset->step_id(attribute(e, "id")); };
    action_t process_text=
      [&](enc_t e) { typeset->text(text(e)); };
    action_t process_multiline_text=
      [&](enc_t e) { typeset->multiline_text(text(e)); };
    action_t process_declare=[&](enc_t e) { typeset->declare(text(e)); };
    action_t process_perform=[&](enc_t e) { typeset->perform(text(e)); };
    action_t process_try=[&](enc_t e) { typeset->try_catch_try(text(e)); };
    action_t process_catch=
      [&](enc_t e) {
        typeset->try_catch_catch(attribute(e, "exception_type"),
                                 text(e),
                                 attribute(e, "success"));
      };
    action_t process_show_value=
      [&](enc_t e) {
        auto expr=get_child(e, "expression1");
        typeset->show_value(text(expr),
                            attribute(expr, "value"));
      };
    action_t process_show_multiline_value=
      [&](enc_t e) {
        auto expr=get_child(e, "expression1");
        typeset->show_multiline_value(text(expr),
                                      attribute(expr, "value"));
      };
    action_t process_with=
      [&](enc_t e) {
        typeset->begin_with(attribute(e, "var"),
                            attribute(e, "container_first"),
                            attribute(e, "container_rest"));
        process(e);
        if (has_attribute(e, "success"))
          typeset->with_summary(attribute(e, "summary"),
                                attribute(e, "n_failed"),
                                attribute(e, "n_total"),
                                attribute(e, "n_errors"),
                                attribute(e, "success"));
        typeset->end_with();
      };
    action_t process_with_results=
      [&](enc_t e) {
        typeset->begin_with_results();
        process(e);
        typeset->end_with_results();
      };
    action_t process_check_true=
      [&](enc_t e) {
        auto expr=get_child(e, "expression1");
        typeset->check_true(text(expr),
                            attribute(e, "success"),
                            attribute(e, "prefix"));
      };
    action_t process_check_true_for=
      [&](enc_t e) {
        auto expr=get_child(e, "expression1");
        auto exprv=get_child(e, "expressionv");
        typeset->check_true_for(text(expr),
                                text(exprv), attribute(exprv, "value"),
                                attribute(e, "success"),
                                attribute(e, "prefix"));
      };
    action_t process_check_equal=
      [&](enc_t e) {
        auto expr1=get_child(e, "expression1");
        auto expr2=get_child(e, "expression2");
        typeset->check_equal(text(expr1), attribute(expr1, "value"),
                             text(expr2), attribute(expr2, "value"),
                             attribute(e, "success"),
                             attribute(e, "prefix"));
      };
    action_t process_check_approx=
      [&](enc_t e) {
        auto expr1=get_child(e, "expression1");
        auto expr2=get_child(e, "expression2");
        typeset->check_approx(text(expr1), attribute(expr1, "value"),
                              text(expr2), attribute(expr2, "value"),
                              attribute(e, "max_error"),
                              attribute(e, "success"),
                              attribute(e, "prefix"));
      };
    action_t process_check_verify=
      [&](enc_t e) {
        auto expr=get_child(e, "expression1");
        auto pred=get_child(e, "predicate");
        typeset->check_verify(text(expr), attribute(expr, "value"),
                              text(pred),
                              attribute(e, "success"),
                              attribute(e, "prefix"));
      };
    action_t process_uncaught_exception=
      [&](enc_t e) { typeset->uncaught_exception(text(e)); };

    auto locate=
      [&](auto f) {
        return
          [&](enc_t e) {
            if (has_attribute(e, "brief_location"))
              typeset->location(attribute(e, "brief_location"));
            f(e);
          };
      };

    actions_t const actions=
      {
#define element(name) {#name, locate(process_##name)}
       element(test),
       element(scope),
       element(declare_scope),
       element(separator),
       element(step_id),
       element(text),
       element(multiline_text),
       element(declare),
       element(perform),
       element(try),
       element(catch),
       element(show_value),
       element(show_multiline_value),
       element(with),
       element(with_results),
       element(check_true),
       element(check_true_for),
       element(check_equal),
       element(check_approx),
       element(check_verify),
       element(uncaught_exception)
#undef element
      };

    process=[&](enc_t e) { process_children(e, actions); };

    auto root=read_element(cin);
    must_be_element(root, "testudo");
    process(root);
  }

}
