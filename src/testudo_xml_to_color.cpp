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
    bool show_lines=true;
    unsigned max_line_length=default_max_line_length;
    while (opts) {
      if (opts.opt("-b"))
        bw=true;
      else if (opts.opt("-s"))
        summary=true;
      else if (opts.opt("-n"))
        show_lines=false;
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
          cout, max_line_length, show_lines);

    using enc_t=element_t::node_const_t;

    // wrap an action with location info
    auto locate=
      [&](action_t f) -> action_t {
        if (f)
          return
            [&, f](enc_t e) {
              if (has_attribute(e, "brief_location"))
                typeset->location(attribute(e, "brief_location"));
              f(e);
            };
        else
          return f;
      };

#define recursive_element(name, open, close)                            \
    {name, {locate(open), close}}
    // terminal elements either have no content, or they handle children
    // explicitly; therefore, they have no need for a "close" action
#define terminal_element(name, open)                                    \
    {name, {locate(open), {}, false}}

    traverse_map_t testudo_xml_to_color_map{

      terminal_element(
        "interactive_test",
        [&](enc_t) { typeset->interactive_test(); }),

      recursive_element("testudo", {}, {}),

      recursive_element(
        "test",
        [&](enc_t e) {
          typeset->title(attribute(e, "location"),
                         attribute(e, "name"),
                         attribute(e, "title"));
        },
        {}),

      terminal_element(
        "stats",
        [&](enc_t e) {
          typeset->summary(attribute(e, "name"),
                           attribute(e, "n_failed"),
                           attribute(e, "n_total"),
                           attribute(e, "n_errors"),
                           attribute(e, "success"));
        }),

      recursive_element(
        "indent",
        [&](enc_t) { typeset->begin_indent(); },
        [&](enc_t) { typeset->end_indent(); }),

      recursive_element(
        "scope",
        [&](enc_t e) { typeset->begin_scope(attribute(e, "name")); },
        [&](enc_t e) { typeset->end_scope(attribute(e, "name")); }),

      recursive_element(
        "declare_scope",
        [&](enc_t e) { typeset->begin_declare_scope(attribute(e, "declare")); },
        [&](enc_t) { typeset->end_declare_scope(); }),

      terminal_element(
        "separator",
        [&](enc_t) { typeset->separator(); }),

      terminal_element(
        "step_id",
        [&](enc_t e) { typeset->step_id(attribute(e, "id")); }),

      terminal_element(
        "text",
        [&](enc_t e) { typeset->text(text(e)); }),

      terminal_element(
        "declare",
        [&](enc_t e) { typeset->declare(text(e)); }),

      terminal_element(
        "perform",
        [&](enc_t e) { typeset->perform(text(e)); }),

      terminal_element(
        "try",
        [&](enc_t e) { typeset->try_catch_try(text(e)); }),

      terminal_element(
        "catch",
        [&](enc_t e) {
          typeset->try_catch_catch(attribute(e, "exception_type"),
                                   text(e),
                                   attribute(e, "success"));
        }),

      terminal_element( // we handle children explicitly
        "show_value",
        [&](enc_t e) {
          auto expr=get_child(e, "expression1");
          typeset->show_value(text(expr), attribute(expr, "value"));
        }),

      recursive_element(
        "with",
        [&](enc_t e) {
          typeset->begin_with(attribute(e, "var"),
                              attribute(e, "container_first"),
                              attribute(e, "container_rest"));
        },
        [&](enc_t) { typeset->end_with(); }),

      terminal_element(
        "with_stats",
        [&](enc_t e) {
          if (has_attribute(e, "success"))
            typeset->with_summary(attribute(e, "name"),
                                  attribute(e, "n_failed"),
                                  attribute(e, "n_total"),
                                  attribute(e, "n_errors"),
                                  attribute(e, "success"));
        }),

      recursive_element(
        "with_results",
        [&](enc_t) { typeset->begin_with_results(); },
        [&](enc_t) { typeset->end_with_results(); }),

      terminal_element( // we handle children explicitly
        "check_true",
        [&](enc_t e) {
          auto expr=get_child(e, "expression1");
          auto exprv=get_child_or_null(e, "expressionv");
          string exprv_str=exprv ? text(exprv) : "";
          string valv_str=exprv ? attribute(exprv, "value") : "";
          auto expl=get_child_or_null(e, "explanation");
          string expl_str=expl ? text(expl) : "";
          typeset->check_true(text(expr),
                              exprv_str, valv_str, expl_str,
                              attribute(e, "success"),
                              attribute(e, "prefix"));
        }),

      terminal_element( // we handle children explicitly
        "check_equal",
        [&](enc_t e) {
          auto expr1=get_child(e, "expression1");
          auto expr2=get_child(e, "expression2");
          auto exprv=get_child_or_null(e, "expressionv");
          string exprv_str=exprv ? text(exprv) : "";
          string valv_str=exprv ? attribute(exprv, "value") : "";
          auto expl=get_child_or_null(e, "explanation");
          string expl_str=expl ? text(expl) : "";
          typeset->check_equal(text(expr1), attribute(expr1, "value"),
                               text(expr2), attribute(expr2, "value"),
                               exprv_str, valv_str, expl_str,
                               attribute(e, "success"),
                               attribute(e, "prefix"));
        }),

      terminal_element( // we handle children explicitly
        "check_approx",
        [&](enc_t e) {
          auto expr1=get_child(e, "expression1");
          auto expr2=get_child(e, "expression2");
          auto exprv=get_child_or_null(e, "expressionv");
          string exprv_str=exprv ? text(exprv) : "";
          string valv_str=exprv ? attribute(exprv, "value") : "";
          auto expl=get_child_or_null(e, "explanation");
          string expl_str=expl ? text(expl) : "";
          typeset->check_approx(text(expr1), attribute(expr1, "value"),
                                text(expr2), attribute(expr2, "value"),
                                attribute(e, "max_error"),
                                exprv_str, valv_str, expl_str,
                                attribute(e, "success"),
                                attribute(e, "prefix"));
        }),

      terminal_element(
        "uncaught_exception",
        [&](enc_t e) { typeset->uncaught_exception(text(e)); })

    };

#undef terminal_element
#undef recursive_element

    try {
      interpret_element(cin, testudo_xml_to_color_map);
    } catch (read_ended_unexpectedly const &excp) {
      typeset->aborted(
        "incomplete XML report; the test probably crashed!");
    }
  }

}
