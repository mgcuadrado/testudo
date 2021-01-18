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

#include "testudo_diff.h"
#include "crc.h"
#include "testudo_typeset_color_text.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <cassert>

using namespace cyclic_redundancy_check;
using namespace std;

namespace testudo___implementation {

  namespace {

    using namespace diff_implementation;

    auto read_track(string file_name) {
      ifstream file(file_name);
      return diff_implementation::parse_track(file);
    }

    string typeset_address(string a) { return a; }
    string typeset_address(string s, string t) {
      string
        s_file=s.substr(0, s.find(':')),
        t_file=s.substr(0, t.find(':'));
      return typeset_address(s+" -> "
                             +((s_file==t_file)
                               ? t.substr(t.find(':')+1)
                               : t));
    }
    string typeset_id(cyclic_redundancy_check::crc64_t id)
      { return "["+to_hex(id)+"]"; }

    // heuristics used for the color of the tag:
    //     >0: for better (green)
    //     <0: for worse (red)
    //      0: same (blue)
    string delta_stats(TestStats const &from, TestStats const &to) {
      auto
        delta_errors=to.n_errors()-from.n_errors(),
        delta_failed=to.n_failed()-from.n_failed(),
        delta_passed=to.n_passed()-from.n_passed();
      // reducing errors is always the best result, and introducing errors is
      // always the worst result:
      if (delta_errors not_eq 0)
        return to_string(-delta_errors);
      // a change in the number of failed is more important than a change in
      // the number of passed:
      if (delta_failed not_eq 0)
        return to_string(-delta_failed);
      // finally, more passed is better:
      if (delta_passed not_eq 0)
        return to_string(+delta_passed);
      return to_string(0);
    }

    string typeset_stats(TestStats const &ts) {
      return
        to_string(ts.n_failed())+"/"
        +to_string(ts.n_passed()+ts.n_failed())+" f"
        +(ts.n_errors()
          ? (", "+to_string(ts.n_errors())+" e")
          : string());
    }

    void output_track(ostream &os, track_summary const &ts,
                      unsigned max_line_length,
                      bool bw) {
    auto typeset=
      (bw ? text_track_typeset : color_text_track_typeset)(
        os, max_line_length);
    for (auto const &[evolution, tracks, is_to]:
           list<tuple<string,
                      list<track_element> const &,
                      bool>>
             {{"deleted good",  ts.deleted_good,  false},
              {"deleted wrong", ts.deleted_wrong, false},
              {"new good",      ts.new_good,      true},
              {"new wrong",     ts.new_wrong,     true}}) {
      if (not tracks.empty()) {
        TestStats e_ts;
        for (auto const &e: tracks)
          e_ts+=e.stats;
        auto e_ts_s=typeset_stats(e_ts);
        typeset->track_header(evolution, to_string(tracks.size()),
                              is_to ? "" : e_ts_s,
                              is_to ? e_ts_s : "",
                              delta_stats(is_to ? TestStats() : e_ts,
                                          is_to ? e_ts : TestStats()));
        for (auto const &e: tracks) {
          auto stats_s=typeset_stats(e.stats);
          typeset->track_entry(typeset_address(e.address),
                               e.type,
                               is_to ? "" : stats_s,
                               is_to ? stats_s : "",
                               delta_stats(is_to ? TestStats() : e.stats,
                                           is_to ? e.stats : TestStats()));
        }
      }
    }
    for (auto const &[evolution, tracks]:
           list<tuple<string,
                      list<pair<track_element, track_element>> const &>>
             {{"wrong to good", ts.wrong_to_good},
              {"good to wrong", ts.good_to_wrong},
              {"with_data changed", ts.with_data_changed}})
      if (not tracks.empty()) {
        TestStats s_ts, t_ts;
        for (auto const &[s, t]: tracks) {
          s_ts+=s.stats;
          t_ts+=t.stats;
        }
        typeset->track_header(evolution, to_string(tracks.size()),
                              typeset_stats(s_ts), typeset_stats(t_ts),
                              delta_stats(s_ts, t_ts));
        for (auto const &[s, t]: tracks) {
          assert(s.type==t.type);
          assert(s.id==t.id);
          typeset->track_entry(typeset_address(s.address, t.address),
                               s.type,
                               typeset_stats(s.stats),
                               typeset_stats(t.stats),
                               delta_stats(s.stats, t.stats));
        }
      }
    }

    track_summary diff(track const &source, track const &target,
                       std::size_t min_length=1) {
      auto track_edit=shortest_edit(source, target, min_length);
      auto table=edit_to_table(track_edit, source.size(), target.size());
      return track_table_to_summary(source, target, table);
    }

  }

  void testudo_diff(opts_t opts) {
    bool bw=false;
    unsigned max_line_length=default_max_line_length;
    string source_filename, target_filename;
    while (opts) {
      if (opts.opt("-b"))
        bw=true;
      else if (auto w=opts.opt_arg("-w"))
        max_line_length=stoi(w);
      else {
        if (source_filename.empty())
          source_filename=opts.arg();
        else if (target_filename.empty())
          target_filename=opts.arg();
        else {
          cerr << opts.executable << ": too many file names" << endl;
          exit(1);
        }
      }
    }
    if (source_filename.empty() or target_filename.empty()) {
      cerr << opts.executable << ": two file names required" << endl;
      exit(1);
    }
    auto
      source=read_track(source_filename),
      target=read_track(target_filename);

    output_track(cout, diff(source, target, 1), max_line_length, bw);
  }

}

namespace testudo___implementation::random {

  size_t random_generator<size_t>::uniform(size_t max)
    { return uniform_int_distribution<size_t>(0, max)(engine); }

}

namespace testudo___implementation::diff_implementation {

  char random_char(random::random_generator<size_t> &rgs,
                   string characters_from)
    { return characters_from[rgs.uniform(characters_from.length()-1)]; }

  string random_string(random::random_generator<size_t> &rgs,
                       size_t length,
                       string characters_from) {
    string result(length, 'x');
    generate(result.begin(), result.end(),
             [&rgs, characters_from]()
               { return random_char(rgs, characters_from); });
    return result;
  }

  string random_diff(random::random_generator<size_t> &rgs,
                     string a, string characters_from) {
    string result;
    size_t i=0;
    while (i<a.length()) {
      char n=random_char(rgs, characters_from);
      result+=n;
      if ((n=='a') or (n=='c'))
        ++i;
    }
    while (true) {
      char n=random_char(rgs, characters_from);
      if ((n=='a') or (n=='c'))
        break;
      result+=n;
    }
    return result;
  }

  string random_patch(random::random_generator<size_t> &rgs,
                      string a, string diff,
                      string characters_from) {
    string result;
    size_t i=0;
    for (char d: diff) {
      switch (d) {
      case 'a': {
        if (i>=a.length())
          throw runtime_error("diff spec referenced char past end");
        ++i;
      } break;
      case 'b': {
        result+=random_char(rgs, characters_from);
      } break;
      case 'c': {
        if (i>=a.length())
          throw runtime_error("diff spec referenced char past end");
        result+=a[i];
        ++i;
      } break;
      default: throw runtime_error(
                 string("character not allowed in diff spec '")+d+"'");
      }
    }
    return result;
  }

}

namespace testudo___implementation::diff_implementation {

  edit_table edit_to_table(edit_t const &e,
                           size_t s_size, size_t t_size) {
    vector<bool> s_matched(s_size, false), t_matched(t_size, false);
    map<size_t, size_t> matches_s_t, matches_t_s;
    {
      size_t j=0, k=0;

      auto read_integer=
        [&e, &k]() {
          std::size_t result=0;
          while ((k<e.length()) and (e[k]>='0') and (e[k]<='9'))
            result=10*result+(e[k++]-'0');
          return result;
        };

      while (k<e.length()) {
        if (j>=t_size)
          throw runtime_error("error: at char "+to_string(k)+", "
                              +to_string(j)+" past target size");
        switch (e[k++]) {
        case 'm': {
            auto l=read_integer();
            if ((l==0) or (k>e.length()) or (e[k++] not_eq ':'))
              throw runtime_error("error: at char "+to_string(k)
                                  +", invalid syntax");
            auto i=read_integer();
            while (l>0) {
              s_matched[i]=t_matched[j]=true;
              matches_s_t[i]=j;
              matches_t_s[j]=i;
              ++i, ++j, --l;
            }
          } break;
        case 'w': {
            auto l=read_integer();
            j+=l;
          } break;
        default:
          throw runtime_error("error: at char "+to_string(k)
                              +", invalid syntax");
        }
        if ((k>=e.length()) or (e[k++] not_eq '.'))
          throw runtime_error("error: at char "+to_string(k)
                              +", invalid syntax");
      }
    }

    edit_table et;
    for (size_t i=0; i<s_size; ++i)
      if (not s_matched[i])
        et.source_disappeared.push_back(i);
    for (size_t j=0; j<t_size; ++j)
      if (not t_matched[j])
        et.target_new.push_back(j);
    copy(matches_s_t.begin(), matches_s_t.end(),
         back_inserter(et.source_matches));
    copy(matches_t_s.begin(), matches_t_s.end(),
         back_inserter(et.target_matches));
    return et;
  }


  bool operator<(track_element const &a, track_element const &b) {
#define compare_field(f) if (a.f not_eq b.f) return a.f<b.f
    // we don't look at the address or at the stats, because they may change
    // from version to version
    compare_field(type);
    compare_field(id);
    return false;
#undef compare_field
  }


  track parse_track(istream &is) {
    track t;
    unsigned long int line_n=0;
    while (is) {
      ++line_n;
      string line;
      getline(is, line);
      if (line.empty())
        continue;
      istringstream issline(line);
      list<string> words{istream_iterator<string>(issline),
                         istream_iterator<string>()};
      if (words.empty())
        throw runtime_error("empty line parsing track");
      track_element e;
      if (words.front()[0]=='[') {
        if (words.front()[words.front().size()-1] not_eq ']')
          throw runtime_error("invalid address parsing track");
        e.address=words.front().substr(1, words.front().size()-2);
        words.pop_front();
      }

      if (words.size()<1)
        throw runtime_error("invalid line "+to_string(line_n)
                            +" parsing track");
      e.type=words.front();
      words.pop_front();
      if (words.empty())
        e.id=0;
      else {
        if (not ((words.front()[0]=='[')
                 and (words.front()[words.front().size()-1]==']')))
          throw runtime_error("invalid ID parsing track: \""+words.front()
                              +"\"");
        e.id=cyclic_redundancy_check::hex_to<crc64_t>(
               words.front().substr(1, words.front().size()-2));
        words.pop_front();
      }

      if (e.type[0]=='c') {
        string stats=words.front();
        words.pop_front();
        if (stats.substr(0, 2) not_eq "r-")
          throw runtime_error("invalid line "+to_string(line_n)
                              +" parsing track");
        stats=stats.substr(1);
        // "stats" is e.g. "-23-2-0"
        istringstream issstats(stats);
        vector<integer> n_stats(3);
        for (int i=0; i<3; ++i) {
          if (issstats.get() not_eq '-')
            throw runtime_error("invalid line "+to_string(line_n)
                                +" parsing track");
          issstats >> n_stats[i];
        }
        if (not issstats.eof())
          throw runtime_error("invalid line "+to_string(line_n)
                              +" parsing track");
        e.stats={n_stats[0], n_stats[1], n_stats[2]};
      }
      if (not words.empty())
        throw runtime_error("invalid line "+to_string(line_n)
                            +" parsing track");
      t.push_back(e);
    }
    return t;
  }

  ostream &operator<<(ostream &os, track_element const &e) {
    if (not e.address.empty())
      os << "[" << typeset_address(e.address) << "] ";
    os << e.type << " " << typeset_id(e.id);
    string stats=
      "r-"+to_string(e.stats.n_passed())
      +"-"+to_string(e.stats.n_failed())
      +"-"+to_string(e.stats.n_errors());
    if (e.type[0]=='c')
      os << " " << stats;
    return os;
  }

  ostream &operator<<(ostream &os, track const &t) {
    for (auto const &e: t)
      os << e << "\n";
    return os;
  }

  namespace {

    bool is_good(TestStats const &ts)
      { return (ts.n_failed()==0) and (ts.n_errors()==0); }

  }

  track_summary track_table_to_summary(track const &source,
                                       track const &target,
                                       edit_table const &table) {
    track_summary ts;

    for (auto i: table.source_disappeared) {
      auto const &te=source[i];
      if (te.type[0]=='c')
        (is_good(te.stats) ? ts.deleted_good : ts.deleted_wrong).push_back(te);
    }
    for (auto j: table.target_new) {
      auto const &e=target[j];
      if (e.type[0]=='c')
        (is_good(e.stats) ? ts.new_good : ts.new_wrong)
          .push_back(e);
    }

    for (auto [i, j]: table.source_matches) {
      auto const &es=source[i], &et=target[j];
      if (es.type[0]=='c') {
        if (es.type=="c-with_summary") {
          if (not (es.stats==et.stats))
            ts.with_data_changed.push_back({es, et});
          // FIXME: but we're missing changes that leave the summary unchanged
        }
        else if (is_good(es.stats) not_eq is_good(et.stats)) {
          (is_good(es.stats) ? ts.good_to_wrong : ts.wrong_to_good)
            .push_back({es, et});
        }
      }
    }

    return ts;
  }

  ostream &operator<<(ostream &os, track_summary const &ts) {
    output_track(os, ts, default_max_line_length, true);
    return os;
  }

}

