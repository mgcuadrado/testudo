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

    string const text_ident=color_ident+"\""+color_normal;
    string const declare_ident=color_ident+":"+color_normal;
    string const declare_end=color_ident+";"+color_normal;
    string const perform_ident=color_ident+"#"+color_normal;
    string const perform_end=color_ident+";"+color_normal;
    string const begin_scope_ident=color_ident+"{"+color_normal;
    string const end_scope_ident=color_ident+"}"+color_normal;
    string const try_ident=color_ident+"&"+color_normal;
    string const try_end=color_ident+"?"+color_normal;
    string const catch_ident=color_ident+"> \""+color_normal;
    string const catch_end=color_ident+"\""+color_normal;
    string const check_ident=color_ident+"%"+color_normal;
    string const check_equal=color_ident+"=="+color_normal;
    string const check_approx=color_ident+"//"+color_normal;
    string const check_max_error=color_ident+"+/-"+color_normal;
    string const show_ident=color_ident+"?"+color_normal;
    string const show_sep=color_ident+":"+color_normal;
    string const show_tab=color_ident+"|"+color_normal;
    string const show_cont="\\";
    string const name_open=color_ident+"{"+color_normal;
    string const name_close=color_ident+"}"+color_normal;

    string const ok_label="["+color_success_tag+" OK "+color_normal+"]";
    string const fail_label="["+color_failure_tag+"FAIL"+color_normal+"]";
    string const error_label="["+color_error+"ERR-"+color_normal+"]";

    constexpr unsigned separator_width=60;
    constexpr unsigned max_line_length=78;
    constexpr unsigned indent=4;

    string::size_type real_length(string s)
      { return (regex_replace(s, color_code, "")).length(); }
    auto real_index(string s, string::size_type i) {
      if (i==0)
        return i;
      if (s.empty())
        return string::npos;
      if (s[0]=='\033') {
        auto end_of_code=s.find('m')+1;
        return real_index(s.substr(end_of_code), i)+end_of_code;
      }
      else
        return real_index(s.substr(1), i-1)+1;
    }

    auto const
      check_length=real_length(ok_label),
      text_line_length=max_line_length-check_length-1;

    auto cut_text(string text) {
      string non_last;
      while (real_length(text)>text_line_length) {
        auto cut_at=real_index(text, text_line_length);
        non_last+=text.substr(0, cut_at)+show_cont+'\n';
        text=string(indent, ' ')+text.substr(cut_at);
      }
      return make_pair(non_last, text);
    }

    void format_text(ostream &stream, string text) {
      auto cut=cut_text(text);
      stream << cut.first << cut.second << endl;;
    }

    void format_label(ostream &stream, string text, char fill, string label) {
      assert(real_length(fail_label)==real_length(ok_label));
      assert(real_length(fail_label)==real_length(error_label));

      auto cut=cut_text(text);
      auto last_length=real_length(cut.second);
      string padding=
        string(" ")+
        ((last_length<text_line_length)
         ? string(text_line_length-last_length-1, fill)
         : "")
        +((last_length+1<text_line_length) ? " " : "");
      stream << cut.first << cut.second
             << color_failure << padding << color_normal
             << label << endl;
    }

    void format_check(ostream &stream, string text, bool check) {
      format_label(stream, text,
                   check ? ' ' : '-',
                   check ? ok_label : fail_label);
    }

    void format_error(ostream &stream, string text)
      { format_label(stream, text, '-', error_label); }

    template <typename... A>
    void regex_in_place_replace(string &s, A &&...a)
      { s=regex_replace(s, a...); }

    string tabify(string s) {
      regex_in_place_replace(s, regex("\n$"), "");
      regex_in_place_replace(s, regex("^|\n"), "$&  "+show_tab+" ");
      return s;
    }

    class TestFormatColorText
    : public TestFormat {
    public:
      ostream &ts;
      TestFormatColorText(ostream &os) : ts(os) { }

      void output_title(string name, string title) override {
        string::size_type total_length=1+name.length()+1+1+title.length();
        ts << color_lines << " _";
        for (unsigned int i=0; i<total_length; i++)
          ts << "_";
        ts << "_" << color_normal;
        ts << endl
               << color_lines << "| " << name_open << name << name_close << " "
               << color_bold << title << color_lines << " |" << color_normal
               << endl;
        ts << color_lines << "`-";
        for (unsigned int i=0; i<total_length; i++)
          ts << "-";
        ts << "-'";
        ts << color_normal << endl;
      }

      void output_begin_scope(string name) {
        format_text(ts, perform_ident+" "+begin_scope_ident
                    +" begin scope "+text_ident+" "+name+" "+text_ident);
      }
      void output_end_scope(string name) {
        format_text(ts, perform_ident+" "+end_scope_ident
                    +" end scope "+text_ident+" "+name+" "+text_ident);
      }

      void output_separator() override {
        ts << color_lines << string(separator_width, '-')
               << color_normal << endl;;
      }

      void output_text(string text) override
        { format_text(ts, text_ident+" "+text+" "+text_ident); }

      void output_multiline_text(string text) override {
        ostringstream ossml;
        ossml.copyfmt(ts);
        ossml << text;
        if (not ossml.str().empty())
          ts << tabify(ossml.str()) << endl;
      }

      void output_declare(string code_str) override
        { format_text(ts, declare_ident+" "+code_str+" "+declare_end); }

      void output_perform(string code_str) override
        { format_text(ts, perform_ident+" "+code_str+" "+perform_end); }

      void output_try(string code_str, string flag_str) override {
        format_text(ts, try_ident+" "+code_str+" "
                    +catch_ident+" " +flag_str+" "+try_end);
      }

      void output_catch(string error) override
        { format_text(ts, catch_ident+" "+error+" "+catch_end); }

      void output_show_value(string expr_str, string value_str) override {
        ostringstream oss;
        oss.copyfmt(ts);
        oss << show_ident << " " << expr_str << " " << show_sep
            << " " << value_str;
        format_text(ts, oss.str());
      }

      void output_show_multiline_value(
          string expr_str, string value_str) override {
        ostringstream oss;
        oss.copyfmt(ts);
        oss << show_ident << " " << expr_str << " " << show_sep;
        format_text(ts, oss.str());
        ostringstream ossml;
        ossml.copyfmt(ts);
        ossml << value_str;
        if (not ossml.str().empty())
          ts << tabify(ossml.str()) << endl;
      }

      void output_check_true(string expr_str, bool expr) override {
        ostringstream oss;
        oss.copyfmt(ts);
        oss << check_ident << " " << expr_str;
        if (not expr)
          oss << " " << show_sep << " false";
        format_check(ts, oss.str(), expr);
      }

      void output_check_equal(string expr1_str, string val1_str,
                              string expr2_str, string val2_str,
                              bool equal) override {
        ostringstream oss;
        oss.copyfmt(ts);
        oss << check_ident << " " << expr1_str << " " << check_equal << " "
            << expr2_str;
        if (not equal)
          oss << " " << show_sep << " "
              << val1_str << " " << check_equal << " " << val2_str;
        format_check(ts, oss.str(), equal);
      }

      void output_check_approx(string expr1_str, string val1_str,
                               string expr2_str, string val2_str,
                               string max_error_str,
                               bool approx) override {
        ostringstream oss;
        oss.copyfmt(ts);
        oss << check_ident << " " << expr1_str << " " << check_approx << " "
            << expr2_str << " " << check_max_error << " " << max_error_str;
        if (not approx)
          oss << " " << show_sep << " "
              << val1_str << " " << check_approx << " " << val2_str;
        format_check(ts, oss.str(), approx);
      }

      void produce_summary(string name, TestStats test_stats) override {
        ostringstream oss;
        oss.copyfmt(ts);
        oss << name_open
            << color_ident << name << color_normal
            << name_close << " ";
        oss << color_ident << test_stats.n_failed << "/"
            << (test_stats.n_failed+test_stats.n_passed)
            << " fail";
        if (test_stats.n_errors)
          oss << ", " << test_stats.n_errors << " err";
        oss << color_normal;
        format_check(ts, oss.str(),
                     (test_stats.n_failed==0) and (test_stats.n_errors==0));
        ts << endl;
      }

      void uncaught_exception(string exception) override {
        format_error(
          ts,
          catch_ident+" uncaught exception: "+exception+" "+catch_end);
      }

      void print_test_readout() const { } // already wrote everything

    private:
      inline static pattern::register_creator<TestFormatColorText>
        rc{test_format_named_creator(), "color_text"};
    };

  }

}
