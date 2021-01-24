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

#include "testudo_typeset_color_text.h"
#include "testudo_format.h"
#include <regex>

namespace {

  using namespace std;
  using namespace testudo___implementation;

  bool non_initial_utf8_char(char c)
    { return (c bitand 0b11000000)==0b10000000; }
  string::size_type skip_non_initial_utf8(string s, string::size_type i) {
    string::size_type l=0;
    while ((i+l<s.length()) and (non_initial_utf8_char(s[i+l])))
      ++l;
    return l;
  }

  struct Color {
    virtual ~Color()=default;
    string
      normal, bold, lines,
      plain_tag, success_tag, failure, failure_tag,
      ident, error;
    string
      step_id_begin, step_id_end,
      text_ident,
      declare_ident, declare_end,
      perform_ident, perform_end,
      begin_scope_ident, end_scope_ident,
      try_ident, catch_ident, catch_begin, catch_end,
      check_ident, check_equal_sign, check_approx_sign, check_max_error,
      check_verify_sign,
      show_ident, show_sep, show_tab, show_cont,
      name_open, name_close,
      loc_begin, loc_end;
    string ok_label, fail_label, error_label;
    string track_loc_begin, track_loc_end;
    string same_label, good_label, bad_label;
    function<string::size_type (string s, string::size_type from)> skip_codes=
      [](string, string::size_type) { return 0; };
    string::size_type real_length(string s) const {
      string::size_type l=0, i=0;
      do {
        i+=skip_non_initial_utf8(s, i);
        i+=skip_codes(s, i);
        if (i==s.length())
          return l;
        ++i;
        ++l;
      } while (true);
    }
    string::size_type real_index(string s, string::size_type i) const {
      string::size_type j=0, k=0;
      do {
        j+=skip_non_initial_utf8(s, j);
        j+=skip_codes(s, j);
        if (k==i)
          return j;
        ++j;
        ++k;
      } while (true);
    }
  };

  void populate_tags(Color &color) {
    auto add_color=[&color](string s) { return color.ident+s+color.normal; };

    color.step_id_begin=add_color("[");
    color.step_id_end=add_color("]");
    color.text_ident=add_color("\"");
    color.declare_ident=add_color(":");
    color.declare_end=add_color(";");
    color.perform_ident=add_color("#");
    color.perform_end=add_color(";");
    color.begin_scope_ident=add_color("{");
    color.end_scope_ident=add_color("}");
    color.try_ident=add_color("&");
    color.catch_ident=add_color(">");
    color.catch_begin=add_color("\"");
    color.catch_end=add_color("\"");
    color.check_ident=add_color("%");
    color.check_equal_sign=add_color("==");
    color.check_approx_sign=add_color("//");
    color.check_max_error=add_color("+/-");
    color.check_verify_sign=add_color("~");
    color.show_ident=add_color("?");
    color.show_sep=add_color(":");
    color.show_tab=add_color("|");
    color.show_cont=color.normal+"\\";
    color.name_open=color.ident+"{";
    color.name_close="}"+color.normal;
    color.loc_begin=color.lines;
    color.loc_end=color.normal;

    color.ok_label="["+color.success_tag+" OK "+color.normal+"]";
    color.fail_label="["+color.failure_tag+"FAIL"+color.normal+"]";
    color.error_label="["+color.error+"ERR-"+color.normal+"]";

    color.track_loc_begin=color.plain_tag+"["+color.normal;
    color.track_loc_end=color.plain_tag+"]"+color.normal;
    color.same_label="["+color.plain_tag+"same"+color.normal+"]";
    color.good_label="["+color.success_tag+"good"+color.normal+"]";
    color.bad_label="["+color.failure_tag+"-BAD-"+color.normal+"]";
  }

  Color const &get_color() {
    static Color color;
    static bool initialized=false;
    if (not initialized) {
      color.normal="\033[0;39m";
      color.bold="\033[1;39m";
      color.lines="\033[0;33m";
      color.plain_tag="\033[0;34m";
      color.success_tag="\033[0;32m";
      color.failure="\033[1;31m";
      color.failure_tag="\033[1;43;31m";
      color.ident="\033[1;34m";
      color.error="\033[1;41;33m";

      color.skip_codes=
        [](string s, string::size_type i) -> string::size_type {
          string::size_type l=0;
          while ((i+l<s.length()) and (s[i+l]=='\033')) {
            while ((i+l<s.length()) and (s[i+l] not_eq 'm'))
              ++l;
            ++l;
          }
          return l;
        };

      populate_tags(color);
      initialized=true;
    }
    return color;
  }

  Color const &get_bw() {
    static Color color;
    static bool initialized=false;
    if (not initialized) {
      populate_tags(color);
      initialized=true;
    }
    return color;
  }

  auto const check_length=
    unsigned(get_color().real_length(get_color().ok_label));

  template <typename... A>
  void regex_in_place_replace(string &s, A &&...a)
    { s=regex_replace(s, a...); }

  bool to_bool(string s) { return TestFormat::to_bool(s); }

  string to_string_delim(list<string> const &ls, string delim) {
    if (ls.empty())
      return "";
    string result="";
    for (auto const &s: ls)
      result+=(&s==&ls.front() ? "": delim)+s;
    return result;
  }

  string stat_summary(string n_failed, string n_total, string n_errors) {
    string result=n_failed+"/"+n_total+" fail";
    if (n_errors not_eq "0")
      result+=", "+n_errors+" err";
    return result;
  }

  class ColorTypeset {
  public:
    ColorTypeset(unsigned max_line_length, bool with_color)
      : color(with_color ? get_color() : get_bw()),
        max_line_length(max_line_length),
        separator_width(max_line_length),
        cut_indent(4),
        text_line_length(max_line_length-check_length-1) { }
    Color const &color;
    unsigned const
      max_line_length, separator_width, cut_indent, text_line_length;

    int indent=0;
    void increase_indent() { ++indent; }
    void decrease_indent() { --indent; }

    string indent_spaces() const
      { return string(max(0, indent*2), ' '); }

    string pad(string::size_type i, char padding=' ') const {
      assert(i<=max_line_length);
      return string(i, padding);
    }

    pair<string, string> cut_line(string text) const {
      string non_last;
      string indentation=indent_spaces();
      text=indentation+text;
      while (color.real_length(text)>text_line_length) {
        auto cut_at=color.real_index(text, text_line_length);
        non_last+=text.substr(0, cut_at);
        // find the last color code in effect, so that it can be restablished
        // at the beginning of each line:
        auto last_color_pos=non_last.rfind("\033[");
        string color_code=
          (last_color_pos==string::npos
           ? string("")
           : non_last.substr(
               last_color_pos,
               non_last.find("m", last_color_pos)+1-last_color_pos));
        if (color_code==color.normal)
          color_code="";
        non_last+=color.show_cont+'\n';
        text=indentation+pad(cut_indent)+color_code+text.substr(cut_at);
      }
      return make_pair(non_last, text);
    }

    string format_text_prefix;
    void set_format_text_prefix(string text) { format_text_prefix=text; }
    void unset_format_text_prefix() { format_text_prefix=""; }

    void format_text(ostream &stream, string text) {
      istringstream text_iss(format_text_prefix+text);
      string line;
      while (getline(text_iss, line)) {
        auto cut=cut_line(line);
        stream << cut.first << cut.second << endl;
      }
      unset_format_text_prefix();
    }

    void format_label(ostream &stream, string text,
                      char fill, string label,
                      string pre_color="", string post_color="") {
      assert(check_length==color.real_length(color.ok_label));
      assert(color.real_length(color.fail_label)==
             color.real_length(color.ok_label));
      assert(color.real_length(color.fail_label)==
             color.real_length(color.error_label));

      // if "fill" is not ' ', we want it to be present at least once;
      // therefore, we append a space to the text
      string pre_padding=(fill==' ' ? "" : " ");
      auto cut=cut_line(format_text_prefix+text+pre_color+pre_padding);
      auto last_length=color.real_length(cut.second);
      string padding=
        ((last_length<text_line_length)
         ? pad(text_line_length-last_length, fill)
         : "")
        +((last_length-1<text_line_length) ? " " : "");
      stream << cut.first << cut.second
             << padding << post_color << label << endl;
      unset_format_text_prefix();
    }

    void format_check(ostream &stream, string text, bool check) {
      if (check)
        format_label(stream, text, ' ', color.ok_label);
      else
        format_label(stream, text, '-', color.fail_label,
                     color.failure, color.normal);
    }

    void format_fill(ostream &stream, char fill,
                     string pre_color, string post_color) {
      stream << format_text_prefix
             << pre_color
             << pad(separator_width-color.real_length(format_text_prefix),
                    fill)
             << post_color << endl;
    }

    void format_error(ostream &stream, string text) {
      format_label(stream, text, '-', color.error_label,
                   color.failure, color.normal);
    }
  };

  class ColorTextReportTypeset
    : public ColorTypeset, public TextTypeset {
  public:
    ColorTextReportTypeset(ostream &os,
                           unsigned max_line_length,
                           bool with_color, bool show_location)
      : ColorTypeset(max_line_length, with_color),
        ts(os),
        show_location(show_location) { }

    ostream &ts;
    bool const show_location;

    string scope_name(string name) const {
      return (name.empty()
              ? ""
              : " "+color.text_ident+" "+name+" "+color.text_ident);
    }

    string tabify(string s) const {
      regex_in_place_replace(s, regex("\n$"), "");
      regex_in_place_replace(s, regex("^|\n"), "$&  "+color.show_tab+" ");
      return s;
    }


    void title(string location, string name, string title) override {
      string title_location=show_location ? location : "";
      string brief_name=
        ((name.length() > title.length())
         and name.substr(name.length()-title.length())==title)
        ? name.substr(0, name.length()-title.length())
        : name;

      ostringstream oss;
      oss << color.name_open << brief_name << color.name_close
          << " " << color.bold << title << color.normal;
      auto cut=cut_line(oss.str());
      auto length=
        max(
          (cut.first.empty()
           ? color.real_length(cut.second)
           : text_line_length+1),
          title_location.length());
      string text=cut.first+cut.second;
      ts << color.lines << " " << pad(length+2, '_') << color.normal
         << endl;
      if (not title_location.empty())
        ts << color.lines << "| "
           << title_location
           << pad(length-title_location.length())
           << " |" << color.normal << endl;
      istringstream cut_first_iss(text);
      string line;
      while (getline(cut_first_iss, line))
        ts << color.lines << "|" << color.normal << " "
           << line << pad(length-color.real_length(line))
           << " " << color.lines << "|" << color.normal << endl;
      ts << color.lines << "`" << pad(length+2, '-') << "'"
         << color.normal << endl;
    }

    void location(string brief_location) override {
      if (show_location and not brief_location.empty())
        set_format_text_prefix(color.loc_begin+brief_location+color.loc_end+" ");
    }

    void begin_scope(string name) override {
      format_text(ts,
                  (color.perform_ident+" "+color.begin_scope_ident
                   +scope_name(name)));
      increase_indent();
    }
    void end_scope(string name) override {
      decrease_indent();
      format_text(ts,
                  (color.perform_ident+" "+color.end_scope_ident
                   +scope_name(name)));
    }

    void begin_declare_scope(string code_str) override {
      format_text(ts,
                  (color.perform_ident+" "+color.begin_scope_ident+" "
                   +color.declare_ident+" "+code_str));
      increase_indent();
    }
    void end_declare_scope() override {
      decrease_indent();
      format_text(ts, (color.perform_ident+" "+color.end_scope_ident));
    }

    void separator() override {
      format_fill(ts, '-', color.lines, color.normal);
    }

    void step_id(string id) override {
      format_text(ts, color.step_id_begin+" "+id+" "+color.step_id_end);
    }

    static bool is_multiline(string text)
      { return (text.find('\n') not_eq string::npos); }

    void text(string text) override {
      if (is_multiline(text)) { // multiline
        ostringstream ossml;
        ossml << text;
        ts << tabify(ossml.str()) << endl;
      }
      else // one line
        format_text(ts, color.text_ident+" "+text+" "+color.text_ident);
    }

    void declare(string code_str) override {
      format_text(ts,
                  (color.declare_ident+" "+code_str+" "+color.declare_end));
    }

    void perform(string code_str) override {
      format_text(ts,
                  (color.perform_ident+" "+code_str+" "+color.perform_end));
    }

    string try_first_part;

    void try_catch_try(string code_str) override {
      assert(try_first_part.empty());
      try_first_part=color.try_ident+" "+code_str;
    }
    void try_catch_catch(string exception_type, string error,
                         string caught) override {
      assert(not try_first_part.empty());
      ostringstream oss;
      oss << try_first_part << " "
          << (exception_type.empty()
              ? string()
              : color.catch_ident+" "+exception_type+" ")
          << color.catch_ident;
      if (caught=="with")
        format_text(ts, oss.str());
      else {
        oss << " " << color.catch_begin << " " << error << " "
            << color.catch_end;
        format_check(ts, oss.str(), to_bool(caught));
      }
      try_first_part="";
    }

    void show_value(string expr_str, string value_str) override {
      ostringstream oss;
      oss << color.show_ident << " " << expr_str << " " << color.show_sep;
      if (is_multiline(value_str)) { // multiline
        format_text(ts, oss.str());
        ostringstream ossml;
        format_text(ossml, value_str);
        ts << tabify(ossml.str()) << endl;
      } else { // one line
        oss << " " << value_str;
        format_text(ts, oss.str());
      }
    }

    void begin_with(string var_name,
                    string container_first, string container_rest) override {
      ostringstream oss;
      oss << color.check_verify_sign << " " << var_name << " in "
          << container_first;
      format_text(ts, oss.str());
      if (not container_rest.empty()) {
        ostringstream ossml;
        format_text(ossml, container_rest);
        ts << tabify(ossml.str()) << endl;
      }
      increase_indent();
    }
    void end_with() override { decrease_indent(); }

    void begin_with_results() override { increase_indent(); }
    void end_with_results() override { decrease_indent(); }

    void summary(string name,
                 string n_failed, string n_total, string n_errors,
                 string success) override {
      with_summary(name, n_failed, n_total, n_errors, success);
      ts << endl;
    }

    void with_summary(string name,
                      string n_failed, string n_total, string n_errors,
                      string success) override {
      ostringstream oss;
      oss << color.name_open << name << color.name_close << " ";
      oss << color.ident << stat_summary(n_failed, n_total, n_errors)
          << color.normal;
      if (n_errors not_eq "0")
        format_error(ts, oss.str());
      else
        format_check(ts, oss.str(), to_bool(success));
    }

    static bool is_any_multiline(list<string> const &texts) {
      for (auto const &text: texts)
        if (is_multiline(text))
          return true;
      return false;
    }

    void output_check_general(string prefix,
                              list<string> const &text, string success,
                              list<string> const &additional_fail_text) {
      string report=color.check_ident+" ";
      if (not prefix.empty())
        report+=color.ident+prefix+color.normal+" ";
      report+=to_string_delim(text, " ");
      if (success=="with")
        format_text(ts, report);
      else {
        bool multiline=is_any_multiline(additional_fail_text);
        if (not to_bool(success) and not multiline)
          report+=
            " "+color.show_sep+" "+to_string_delim(additional_fail_text, " ");
        format_check(ts, report, to_bool(success));
        if (not to_bool(success) and multiline)
        for (auto const &text: additional_fail_text)
          ts << tabify(text) << endl;
      }
    }

    void check_true(string expr_str, string success, string prefix) override {
      output_check_general(prefix, {expr_str}, success, {"false"});
    }

    void check_true_for(string expr_str,
                        string exprv_str, string valv_str,
                        string success,
                        string prefix) override {
      output_check_general(
        prefix,
        {expr_str},
        success,
        {"false", color.show_ident, exprv_str, color.show_sep, valv_str});
    }

    void check_equal(string expr1_str, string val1_str,
                     string expr2_str, string val2_str,
                     string success,
                     string prefix) override {
      output_check_general(prefix,
                           {expr1_str, color.check_equal_sign, expr2_str},
                           success,
                           {val1_str, color.check_equal_sign, val2_str});
    }

    void check_approx(string expr1_str, string val1_str,
                      string expr2_str, string val2_str,
                      string max_error_str,
                      string success,
                      string prefix) override {
      output_check_general(
        prefix,
        {expr1_str, color.check_approx_sign, expr2_str,
         color.check_max_error, max_error_str},
        success,
        {val1_str, color.check_approx_sign, val2_str});
    }

    void check_verify(string expr_str, string val_str,
                      string pred_str,
                      string success,
                      string prefix) override {
      output_check_general(prefix,
                           {expr_str, color.check_verify_sign, pred_str},
                           success,
                           {val_str});
    }

    void uncaught_exception(string exception) override {
      format_error(ts,
                   (color.catch_ident+" uncaught exception "
                    +color.catch_begin+" "+exception+" "+color.catch_end));
    }
  };

  class ColorTextSummaryTypeset
    : public ColorTypeset, public TextTypeset {
  public:
    ColorTextSummaryTypeset(ostream &os,
                            unsigned max_line_length, bool with_color)
      : ColorTypeset(max_line_length, with_color),
        ts(os)
      { decrease_indent(); }

    ostream &ts;

    list<pair<string, ostringstream>> test_stack;

    void title(string, string name, string) override {
      test_stack.emplace_back();
      test_stack.back().first=name;
      increase_indent();
    }
    void summary(string name,
                 string n_failed, string n_total, string n_errors,
                 string success) override {
      decrease_indent();
      assert(test_stack.back().first==name);

      if (auto last_dot=name.rfind('.'); last_dot not_eq string::npos)
        name="-"+name.substr(last_dot);
      string summary_line=
        color.ident+"{"+color.normal
        +name
        +color.ident+"}"+color.normal
        +" "+stat_summary(n_failed, n_total, n_errors);
      ostringstream this_summary;
      if (n_errors not_eq "0")
        format_error(this_summary, summary_line);
      else
        format_check(this_summary, summary_line, to_bool(success));

      string accumulated=this_summary.str()+test_stack.back().second.str();
      test_stack.pop_back();
      if (test_stack.empty())
        ts << accumulated;
      else
        test_stack.back().second << accumulated;
    }

    void location(string) { }
    void begin_scope(string) { }
    void end_scope(string) { }
    void begin_declare_scope(string) { }
    void end_declare_scope() { }
    void separator() { }
    void step_id(string) { }
    void text(string) { }
    void multiline_text(string) { }
    void declare(string) { }
    void perform(string) { }
    void try_catch_try(string) { }
    void try_catch_catch(string, string, string) { }
    void show_value(string, string) { }
    void show_multiline_value(string, string) { }
    void begin_with(string, string, string) { }
    void end_with() { }
    void begin_with_results() { }
    void end_with_results() { }
    void with_summary(string, string, string, string, string) { }
    void check_true(string, string, string) { }
    void check_true_for(string, string, string, string, string) { }
    void check_equal(string, string, string, string, string, string) { }
    void check_approx(string, string, string, string, string, string,
                      string) { }
    void check_verify(string, string, string, string, string) { }
    void uncaught_exception(string) { }

  };


  class ColorTextTrackTypeset
    : public ColorTypeset, public TextTrackTypeset {
  public:
    ColorTextTrackTypeset(ostream &os,
                          unsigned max_line_length,
                          bool with_color)
      : ColorTypeset(max_line_length, with_color),
        ts(os) { }

    ostream &ts;

    static string typeset_from_to(string from, string to) {
      if (from.empty())
        return to;
      else if (to.empty())
        return from;
      else
        return from+" -> "+to;
    }

    string delta_tag(string delta_stats) const {
      auto delta_stats_i=stoll(delta_stats);
      return
        (delta_stats_i<0) ? color.bad_label
        : (delta_stats_i>0) ? color.good_label
        : color.same_label;
    }

    string delta_color(string delta_stats) const {
      auto delta_stats_i=stoll(delta_stats);
      return
        (delta_stats_i<0) ? color.failure_tag
        : (delta_stats_i>0) ? color.success_tag
        : color.ident;
    }

    void no_changes() override {
      format_text(ts,
                  (color.same_label+" "+color.plain_tag
                   +"no progress and no regress"+color.normal));
    }

    void track_header(string evolution, string count,
                      string stats_from, string stats_to,
                      string delta_stats) override {
      format_text(ts,
                  (delta_tag(delta_stats)+" "
                   +delta_color(delta_stats)
                   +evolution
                   +" ("+count+": "
                   +typeset_from_to(stats_from, stats_to)
                   +")"+color.normal));
    }

    void track_entry(string location, string type,
                     string stats_from, string stats_to,
                     string delta_stats, bool show_label) {
      increase_indent();
      format_text(ts,
                  (show_label ? delta_tag(delta_stats)+" " : "")
                  +color.track_loc_begin+color.lines+location+color.normal
                  +color.track_loc_end
                  +" "+type+" "
                  +delta_color(delta_stats)+"("
                  +typeset_from_to(stats_from, stats_to)
                  +")"+color.normal);
      decrease_indent();
    }
  };

}

namespace testudo___implementation {

  string::size_type utf8_length(string const &s) {
    auto length=s.length();
    for (auto c: s)
      if (non_initial_utf8_char(c))
        --length;
    return length;
  }

  shared_ptr<TextTypeset>
  text_report_typeset(ostream &os,
                      unsigned max_line_length, bool show_location) {
    return make_shared<ColorTextReportTypeset>(
             os, max_line_length, false, show_location);
  }
  shared_ptr<TextTypeset>
  color_text_report_typeset(ostream &os,
                            unsigned max_line_length, bool show_location) {
    return make_shared<ColorTextReportTypeset>(
             os, max_line_length, true, show_location);
  }

  shared_ptr<TextTypeset>
  text_summary_typeset(ostream &os,
                       unsigned max_line_length) {
    return make_shared<ColorTextSummaryTypeset>(
             os, max_line_length, false);
  }
  shared_ptr<TextTypeset>
  color_text_summary_typeset(ostream &os,
                             unsigned max_line_length) {
    return make_shared<ColorTextSummaryTypeset>(
             os, max_line_length, true);
  }

  shared_ptr<TextTrackTypeset>
  text_track_typeset(ostream &os, unsigned max_line_length)
    { return make_shared<ColorTextTrackTypeset>(os, max_line_length, false); }
  shared_ptr<TextTrackTypeset>
  color_text_track_typeset(ostream &os, unsigned max_line_length)
    { return make_shared<ColorTextTrackTypeset>(os, max_line_length, true); }
}
