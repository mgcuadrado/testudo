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
#include <ostream>
#include <regex>
#include <cassert>

namespace testudo {

  using namespace std;

  namespace {

    regex const color_code("\033\\[[^m]*m");

    string const color_normal="\033[0;39m";
    string const color_bold="\033[1;39m";
    string const color_lines="\033[0;33m";
//     string const color_success="\033[1;32m";
    string const color_success_tag="\033[0;32m";
//     string const color_failure="\033[1;31m";
    string const color_failure="\033[1;31m";
    string const color_failure_tag="\033[1;43;31m";
    string const color_ident="\033[1;34m";
    string const color_error="\033[1;41;33m";

    string add_color(string s) { return color_ident+s+color_normal; }

    string const
      step_id_begin=add_color("["),
      step_id_end=add_color("]"),
      text_ident=add_color("\""),
      declare_ident=add_color(":"),
      declare_end=add_color(";"),
      perform_ident=add_color("#"),
      perform_end=add_color(";"),
      begin_scope_ident=add_color("{"),
      end_scope_ident=add_color("}"),
      try_ident=add_color("&"),
      catch_ident=add_color(">"),
      catch_begin=add_color("\""),
      catch_end=add_color("\""),
      check_ident=add_color("%"),
      check_equal=add_color("=="),
      check_approx=add_color("//"),
      check_max_error=add_color("+/-"),
      check_verify=add_color("~"),
      show_ident=add_color("?"),
      show_sep=add_color(":"),
      show_tab=add_color("|"),
      show_cont=color_normal+"\\",
      name_open=color_ident+"{",
      name_close="}"+color_normal;

    string const
      ok_label="["+color_success_tag+" OK "+color_normal+"]",
      fail_label="["+color_failure_tag+"FAIL"+color_normal+"]",
      error_label="["+color_error+"ERR-"+color_normal+"]";

    constexpr unsigned max_line_length=78;
    constexpr unsigned separator_width=max_line_length;
    constexpr unsigned cut_indent=4;

    string::size_type real_length(string s)
      { return (regex_replace(s, color_code, "")).length(); }
    string::size_type real_index(string s, string::size_type i) {
      if (s[0]=='\033') {
        auto end_of_code=s.find('m')+1;
        return real_index(s.substr(end_of_code), i)+end_of_code;
      }
      else if (i==0)
        return i;
      else if (s.empty())
        return string::npos;
      else
        return real_index(s.substr(1), i-1)+1;
    }

    auto const
      check_length=real_length(ok_label),
      text_line_length=max_line_length-check_length-1;

    template <typename... A>
    void regex_in_place_replace(string &s, A &&...a)
      { s=regex_replace(s, a...); }

    class TestFormatColorText
    : public TestFormat {
    public:
      ostream &ts;
      TestFormatColorText(ostream &os) : ts(os) { }

    private:
      unsigned indent=0;
      void increase_indent() { ++indent; }
      void decrease_indent() { --indent; }

      string indent_spaces() const { return string(indent*2, ' '); }

      auto cut_line(string text) const {
        string non_last;
        string indentation=indent_spaces();
        text=indentation+text;
        while (real_length(text)>text_line_length) {
          auto cut_at=real_index(text, text_line_length);
          non_last+=text.substr(0, cut_at);
          auto last_color_pos=non_last.rfind("\033[");
          // find the last color code in effect, so that it can be restablished
          // at the beginning of each line:
          string color=
            (last_color_pos==string::npos
             ? string("")
             : non_last.substr(
                 last_color_pos,
                 non_last.find("m", last_color_pos)+1-last_color_pos));
          if (color==color_normal)
            color="";
          non_last+=show_cont+'\n';
          text=indentation+string(cut_indent, ' ')+color+text.substr(cut_at);
        }
        return make_pair(non_last, text);
      }

      void format_text(ostream &stream, string text) {
        istringstream text_iss(text);
        string line;
        while (getline(text_iss, line)) {
          auto cut=cut_line(line);
          stream << cut.first << cut.second << endl;
        }
      }

      void format_label(ostream &stream, string text,
                        char fill, string label,
                        string pre_color="", string post_color="") const {
        assert(real_length(fail_label)==real_length(ok_label));
        assert(real_length(fail_label)==real_length(error_label));

        // if "fill" is not ' ', we want it to be present at least once;
        // therefore, we append a space to the text
        string pre_padding=(fill==' ' ? "" : " ");
        auto cut=cut_line(text+pre_color+pre_padding);
        auto last_length=real_length(cut.second);
        string padding=
          ((last_length<text_line_length)
           ? string(text_line_length-last_length, fill)
           : "")
          +((last_length-1<text_line_length) ? " " : "");
        stream << cut.first << cut.second
               << padding << post_color << label << endl;
      }

      void format_check(ostream &stream, string text, bool check) const {
        if (check)
          format_label(stream, text, ' ', ok_label);
        else
          format_label(stream, text, '-', fail_label,
                       color_failure, color_normal);
      }

      void format_error(ostream &stream, string text) const {
        format_label(stream, text, '-', error_label,
                     color_failure, color_normal);
      }

      string tabify(string s) const {
        regex_in_place_replace(s, regex("\n$"), "");
        regex_in_place_replace(s, regex("^|\n"), "$&  "+show_tab+" ");
        return s;
      }


    public:

      void output_title(string name, string title) override {
        ostringstream oss;
        oss << name_open << name << name_close
            << " " << color_bold << title << color_normal;
        auto cut=cut_line(oss.str());
        auto length=
          cut.first.empty() ? real_length(cut.second) : text_line_length+1;
        string text=cut.first+cut.second;
        ts << color_lines << " " << string(length+2, '_') << color_normal;
        ts << endl;
        istringstream cut_first_iss(text);
        string line;
        while (getline(cut_first_iss, line))
          ts << color_lines << "|" << color_normal << " "
             << line << string(length-real_length(line), ' ')
             << " " << color_lines << "|" << color_normal << endl;
        ts << color_lines << "`" << string(length+2, '-') << "'";
        ts << color_normal << endl;
      }

      void output_begin_scope(string name) {
        format_text(ts,
                    (perform_ident+" "+begin_scope_ident
                     +(name.empty()
                       ? ""
                       : " "+text_ident+" "+name+" "+text_ident)));
        increase_indent();
      }
      void output_end_scope(string name) {
        decrease_indent();
        format_text(ts,
                    (perform_ident+" "+end_scope_ident
                     +(name.empty()
                       ? ""
                       : " "+text_ident+" "+name+" "+text_ident)));
      }

      void output_begin_declare_scope(string code_str) {
        format_text(ts,
                    (perform_ident+" "+begin_scope_ident+" "+declare_ident+" "
                     +code_str));
        increase_indent();
      }
      void output_end_declare_scope() {
        decrease_indent();
        format_text(ts, perform_ident+" "+end_scope_ident);
      }

      void output_separator() override {
        ts << color_lines << string(separator_width, '-')
               << color_normal << endl;;
      }

      void output_step_id(string id) override
        { format_text(ts, step_id_begin+" "+id+" "+step_id_end); }

      void output_text(string text) override
        { format_text(ts, text_ident+" "+text+" "+text_ident); }

      void output_multiline_text(string text) override {
        ostringstream ossml;
        ossml << text;
        if (not ossml.str().empty())
          ts << tabify(ossml.str()) << endl;
      }

      void output_declare(string code_str) override
        { format_text(ts, declare_ident+" "+code_str+" "+declare_end); }

      void output_perform(string code_str) override
        { format_text(ts, perform_ident+" "+code_str+" "+perform_end); }

      string try_first_part;

      void output_try(string code_str) override {
        assert(try_first_part.empty());
        try_first_part=try_ident+" "+code_str;
      }

      void output_catch(string exception_type, string error,
                        string caught) override {
        assert(not try_first_part.empty());
        ostringstream oss;
        oss << try_first_part << " "
            << (exception_type.empty()
                ? string()
                : catch_ident+" "+exception_type+" ")
            << catch_ident;
        if (caught=="with")
          format_text(ts, oss.str());
        else {
          oss << " " << catch_begin << " " << error << " " << catch_end;
          format_check(ts, oss.str(), to_bool(caught));
        }
        try_first_part="";
      }

      void output_show_value(string expr_str, string value_str) override {
        ostringstream oss;
        oss << show_ident << " " << expr_str << " " << show_sep
            << " " << value_str;
        format_text(ts, oss.str());
      }

      void output_show_multiline_value(
          string expr_str, string value_str) override {
        ostringstream oss;
        oss << show_ident << " " << expr_str << " " << show_sep;
        format_text(ts, oss.str());
        ostringstream ossml;
        format_text(ossml, value_str);
        if (not ossml.str().empty())
          ts << tabify(ossml.str()) << endl;
      }

      void output_begin_with(string var_name, string container) override {
        ostringstream oss;
        oss << check_verify << " " << var_name << " in " << container;
        format_text(ts, oss.str());
        increase_indent();
      }

      void output_end_with() override { decrease_indent(); }

      void output_begin_with_results() override { increase_indent(); }

      void output_end_with_results() override { decrease_indent(); }

      void output_summary(string name, TestStats test_stats) {
        ostringstream oss;
        oss << name_open << name << name_close << " ";
        oss << color_ident << test_stats.n_failed() << "/"
            << (test_stats.n_failed()+test_stats.n_passed())
            << " fail";
        if (test_stats.n_errors())
          oss << ", " << test_stats.n_errors() << " err";
        oss << color_normal;
        if (test_stats.n_errors())
          format_error(ts, oss.str());
        else
          format_check(
            ts, oss.str(),
            (test_stats.n_failed()==0) and (test_stats.n_errors()==0));
      }

      void output_with_summary(string name, TestStats test_stats) override
        { output_summary(name, test_stats); }

      string to_string_delim(list<string> const &ls, string delim) {
        if (ls.empty())
          return "";
        string result="";
        for (auto const &s: ls)
          result+=(&s==&ls.front() ? "": delim)+s;
        return result;
      }

      void output_check_general(string prefix,
                                list<string> const &text, string success,
                                list<string> const &additional_fail_text) {
        string report=check_ident+" ";
        if (not prefix.empty())
          report+=add_color(prefix)+" ";
        report+=to_string_delim(text, " ");
        if (success=="with")
          format_text(ts, report);
        else {
          if (not to_bool(success))
            report+=
              " "+show_sep+" "+to_string_delim(additional_fail_text, " ");
          format_check(ts, report, to_bool(success));
        }
      }

      void output_check_true(string expr_str, string success,
                             string prefix) override {
        output_check_general(prefix, {expr_str}, success, {"false"});
      }

      void output_check_equal(string expr1_str, string val1_str,
                              string expr2_str, string val2_str,
                              string success,
                              string prefix) override {
        output_check_general(prefix,
                             {expr1_str, check_equal, expr2_str},
                             success,
                             {val1_str, check_equal, val2_str});
      }

      void output_check_approx(string expr1_str, string val1_str,
                               string expr2_str, string val2_str,
                               string max_error_str,
                               string success,
                               string prefix) override {
        output_check_general(
          prefix,
          {expr1_str, check_approx, expr2_str, check_max_error, max_error_str},
          success,
          {val1_str, check_approx, val2_str});
      }

      void output_check_verify(std::string expr_str, std::string val_str,
                               std::string pred_str,
                               string success,
                               string prefix) override {
        output_check_general(prefix,
                             {expr_str, check_verify, pred_str},
                             success,
                             {val_str});
      }

      void produce_summary(string name, TestStats test_stats) override {
        output_summary(name, test_stats);
        ts << endl;
      }

      void uncaught_exception(string exception) override {
        format_error(ts,
                     catch_ident+" uncaught exception "
                     +catch_begin+" "+exception+" "+catch_end);
      }

      void print_test_readout() const { } // already wrote everything

    private:
      inline static pattern::register_creator<TestFormatColorText>
        rc{test_format_named_creator(), "color_text"};
    };

  }

}
