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

#ifndef MGCUADRADO_TESTUDO_DIFF_HEADER_
#define MGCUADRADO_TESTUDO_DIFF_HEADER_

#include "testudo_opt.h"
#include "testudo_stats.h"
#include "crc.h"
#include <string>
#include <random>
#include <cassert>
#include <unordered_map>
#include <map>
#include <set>

#define DO_TRACE
#include "trace.h"

namespace testudo___implementation {

  void testudo_diff(opts_t);

  namespace diff_implementation {

    using lcs_t=std::string; // 'a': first file; 'b': second file; 'c': common

    template <typename Container>
    bool check_common_subsequence(Container const &a, Container const &b,
                                  lcs_t lcs) {
      // for different chunks, all 'a's before any 'b'
      if (lcs.find("ba") not_eq std::string::npos)
        return false;
      size_t i=0, j=0;
      for (auto s: lcs) {
        switch (s) {
        case 'a': ++i; break;
        case 'b': ++j; break;
        case 'c': {
            if (a[i] not_eq b[j])
              return false;
            ++i;
            ++j;
          } break;
        default: return false;
        }
        if ((i>a.size()) or (j>b.size()))
          return false;
      }
      return (i==a.size()) and (j==b.size());
    }

    // "http://simplygenius.net/Article/DiffTutorial1" (see also
    // ".../DiffTutorial2")
    template <typename Container>
    lcs_t longest_common_subsequence_debug(
        Container const &a, Container const &b) {
      // a_i≡a[i-1]; b_i≡b[i-1]
      using integer=std::make_signed_t<std::size_t>;
      integer n=a.size(), m=b.size();
      integer max=m+n, base=max; // "base" is the base for "v" indices
      std::list<std::vector<integer>> vs;
      {
        std::vector<integer> v(2*max+1); // v_i≡v[base+i]
        v[base+1]=0;
        bool found=false;
        for (integer d=0; (d<=max) and not found; ++d) {
          for (integer k=-d; (k<=+d) and not found; k+=2) {
            bool down=(k==-d) or ((k not_eq d) and (v[base+k-1]<v[base+k+1]));
            [[maybe_unused]] integer
              k_prev=down ? k+1 : k-1,
              x_start=v[base+k_prev],
              y_start=x_start-k_prev,
              x_mid=down ? x_start : x_start+1,
              y_mid=x_mid-k,
              x_end=x_mid,
              y_end=y_mid,
              snake_length=0;
            while ((x_end<n) and (y_end<m) and (a[x_end]==b[y_end]))
              ++x_end, ++y_end, ++snake_length;
            v[base+k]=x_end;
            if ((x_end>=n) and (y_end>=m))
              found=true;
          }
          vs.push_back(v);
        }
        assert(found);
      }

      lcs_t result;
      integer x=n, y=m;
      auto vs_it=vs.rbegin();
      for (integer d=vs.size()-1; (x>0) or (y>0); --d, ++vs_it) {
        assert(d>=0);
        auto const &v=*vs_it;
        [[maybe_unused]] integer
          k=x-y,
          x_end=v[base+k],
          y_end=x-k;
        bool down=(k==-d) or ((k not_eq d) and (v[base+k-1]<v[base+k+1]));
        [[maybe_unused]] integer
          k_prev=down ? k+1 : k-1,
          x_start=v[base+k_prev],
          y_start=x_start-k_prev,
          x_mid=down ? x_start : x_start+1,
          y_mid=x_mid-k,
          snake_length=x_end-x_mid;
        assert(snake_length>=0);
        x=x_start;
        y=y_start;
        std::string square=(d>0) ? (down ? "b" : "a") : "";
        std::string diagonal(snake_length, 'c');
        result=square+diagonal+result;
      }
      return result;
    }

    template <typename Container>
    lcs_t longest_common_subsequence(Container const &a, Container const &b) {
      auto result=longest_common_subsequence_debug(a, b);
      assert(check_common_subsequence(a, b, result));
      return result;
    }

  }

  namespace random {

    template <typename T=void>
    struct random_generator;

    template <>
    struct random_generator<> {
      std::mt19937_64 engine;
    };

    template <>
    struct random_generator<std::size_t>
      : public random_generator<> {
      std::size_t uniform(std::size_t max);
    };

  }

  namespace diff_implementation {

    char random_char(random::random_generator<std::size_t> &,
                     std::string characters_from);
    std::string random_string(random::random_generator<size_t> &rgs,
                              std::size_t length,
                              std::string characters_from);
    std::string random_diff(random::random_generator<size_t> &,
                            std::string a, std::string characters_from);
    std::string random_patch(random::random_generator<size_t> &,
                             std::string a, std::string diff,
                             std::string characters_from);

  }

  namespace diff_implementation {

    // "w12.m9:14.w2." means "the targed sequence is formed by 12 new elements,
    // followed by 9 elements taken from the original one starting at 14
    // (0-based), then 2 new elements"
    using edit_t=std::string;

    template <typename value_t>
    bool equal_value(value_t const &a, value_t const &b)
      { return ((not (a<b)) and (not (b<a))); }

    template <typename Container>
    bool check_edit(Container const &s, Container const &t, edit_t e) {
      std::size_t j=0, k=0;

      auto read_integer=
        [&e, &k]() {
          std::size_t result=0;
          while ((k<e.length()) and (e[k]>='0') and (e[k]<='9'))
            result=10*result+(e[k++]-'0');
          return result;
        };

      std::vector<bool> av_s(s.size(), true); // not-yet-usedness ("available")
      while (k<e.length()) {
        if (j>=t.size())
          return false; // ran out of 't'
        switch (e[k++]) {
        case 'm': {
            auto l=read_integer();
            if ((l==0) or (k>=e.length()) or (e[k++] not_eq ':'))
              return false; // invalid length (0, non-number, not ended by ':')
            auto i=read_integer();
            while (l>0) { // check the common sequence
              if ((i>s.size()) or (j>t.size())
                  or (not equal_value(s[i], t[j])) or (not av_s[i]))
                return false; // ran out of 's' or 't', or used already used 's'
              av_s[i]=false; // mark as used
              ++i, ++j, --l;
            }
          } break;
        case 'w': {
            auto l=read_integer();
            if (l==0)
              return false; // invalid length(0, non-number)
            j+=l; // advance past the 't'-specific part
            // if (j>t.size())
            //   return false; // ran out of 't'
          } break;
        default: return false; // invalid char (only 'm' and 'w' are valid)
        }
        if ((k>=e.length()) or (e[k++] not_eq '.'))
          return false; // 'm' or 'w' structure not ended by ':'
      }
      return (j==t.size()); // did we exhaust 't' exactly?
    }

    // greedy algorithm; inspired by (but different from, since i don't want to
    // consider copy operations, and want to find longest sequences first,
    // wherever they start) "http://docs.lib.purdue.edu/cstech/378" (The
    // String-to-String Correction Problem with Block Moves" by Walter
    // F. Tichy); common blocks are considered only if their length is at least
    // "min_length"
    //
    // possible improvements:
    //
    //     * prefer "close" matches rather than leftmost ones; "close" can be
    //       defined as "close to the place the current match occupies between
    //       the one on the left and the one on the right", considering only
    //       already used matches
    //
    //     * use subsequence matches coming from bigger sequences before pure
    //       matches with the same length (they match their surroundings)
    DEFINE_TRACE(shortest_edit);
    template <typename Container>
    edit_t shortest_edit_debug(Container const &s, Container const &t,
                               std::size_t min_length=1) {
      LOCAL_TRACE(shortest_edit);
      LTRACE(trace << "s:\"" << s << "\"" << endl);
      LTRACE(trace << "t:\"" << t << "\"" << endl);
      using value_t=typename Container::value_type;
      using integer=std::size_t;

      // unordered_map<t_index, <length, s_index>>
      std::unordered_map<integer, std::pair<integer, integer>> moves;

      // an index of 's' element sequences of length "min_length"
      std::multimap<std::vector<value_t>, integer> s_index;
      for (integer i=0; i<s.size()-(min_length-1); ++i) {
        std::vector<value_t> s_seq(s.begin()+i, s.begin()+i+min_length);
        s_index.insert({s_seq, i});
      }

      // map<i, list<pair<i, i>>>?
      std::map<integer, std::set<std::pair<integer, integer>>,
               std::greater<integer>>
        longest_l_i_j;
      // search for sequences in 's' that match 't' starting at each location
      for (integer j=0; j<t.size()-(min_length-1); ++j) {
        std::vector<value_t> t_seq(t.begin()+j, t.begin()+j+min_length);
        auto s_r=s_index.equal_range(t_seq);
        // measure common sequence lengths starting at all locations in 's'
        // that match current 't' element
        for (auto s_r_it=s_r.first; s_r_it not_eq s_r.second; ++s_r_it) {
          integer i=s_r_it->second;
          assert(equal_value(s[i], t[j]));
          if ((i>0) and (j>0) and equal_value(s[i-1], t[j-1]))
            continue; // just a subsequence of a longer sequence
          integer length=0;
          integer i_check=i, j_check=j;
          while((i_check<s.size()) and (j_check<t.size())
                and equal_value(s[i_check], t[j_check]))
            ++length, ++i_check, ++j_check;
          if (length>=min_length) {
            longest_l_i_j[length].insert({i, j});
            LTRACE(trace << "  seq " << length << "@" << i << ":" << j
                         << endl);
          }
        }
      }

      // not-yet-usedness ("available"):
      std::vector<bool> av_s(s.size(), true), av_t(t.size(), true);

      while (not longest_l_i_j.empty()) {
        auto const &[length, longest_i_j]=*longest_l_i_j.begin();
        // when considering common sequences with the longest length, we
        // favor the leftmost one (through the order built-in in
        // "longest_i_j")
        for (auto [i_start, j_start]: longest_i_j) {
          LTRACE(trace << "  check " << length
                       << "@" << i_start << ":" << j_start << endl);
          // "l" will span the whole sequence; "s_l" is a subsequence length
          // counter, in case the whole sequence isn't available
          integer i=i_start, j=j_start, l=0, s_l=0;
          while (l<=length) { // one past the length
            if (s_l==length) { // is the whole sequence valid?
              moves[j_start]={length, i_start};
              LTRACE(trace << "    move " << length
                           << "@" << i_start << ":" << j_start << endl);
              // mark used 's' and 't' elements
              for (integer l=0; l<length; ++l)
                av_s[i_start+l]=av_t[j_start+l]=false;
            }
            else if ((l<length) and (av_s[i] and av_t[j]))
              ++s_l; // available element
            else { // end of valid subsequence
              if (s_l>=min_length) { // a long enough subsequence
                longest_l_i_j[s_l].insert({i-s_l, j-s_l});
                LTRACE(trace << "  subseq " << s_l << "@"
                             << i-s_l << ":" << j-s_l << endl);
              }
              s_l=0; // reset subsequence length
            }
            LTRACE(trace << "    " << l << " " << s_l << " "
                         << i << "=" << av_s[i] << " " << j << "=" << av_s[j]
                         << endl);
            ++i, ++j, ++l;
          }
        }
        longest_l_i_j.erase(longest_l_i_j.begin());
      }

      // now, we decode "moves" into an edit string, using "av_t" to know which
      // 't' elements have been matched
      edit_t result;
      for (integer j=0; j<t.size(); ) {
        if (av_t[j]) { // unused 't' element; start a sequence of 'w'
          // how many unused 't' elements in a row?
          integer l=0;
          while ((j<t.size()) and av_t[j])
            ++j, ++l;
          result+="w"+std::to_string(l)+".";
        } else { // used 't' element; there is a common sequence in "moves"
          auto [l, i]=moves.at(j);
          assert(l>0);
          j+=l; // advance to the end of the common sequence
          result+="m"+std::to_string(l)+":"+std::to_string(i)+".";
        }
      }
      return result;
    }

    template <typename Container>
    edit_t shortest_edit(Container const &s, Container const &t,
                         std::size_t min_length=1) {
      auto result=shortest_edit_debug(s, t, min_length);
      assert(check_edit(s, t, result));
      return result;
    }

    struct edit_table {
      std::vector<std::size_t>
        source_disappeared, // ordered by source index
        target_new; // ordered by target index
      std::vector<std::pair<std::size_t, std::size_t>>
        source_matches, // ordered by source index
        target_matches; // ordered by source index (same info as previous one)
    };

    edit_table edit_to_table(edit_t const &e,
                             std::size_t s_length, std::size_t t_length);


    struct track_element {
      std::string address;
      std::string type;
      cyclic_redundancy_check::crc64_t id;
      testudo::TestStats stats;
    };
    bool operator<(track_element const &, track_element const &);
    std::ostream &operator<<(std::ostream &, track_element const &);

    using track=std::vector<track_element>;

    track parse_track(std::istream &is);
    std::ostream &operator<<(std::ostream &, track const &);

    struct track_summary {
      std::list<track_element>
        deleted_good,
        deleted_wrong,
        new_good,
        new_wrong,
        new_error;
      std::list<std::pair<track_element, track_element>>
        wrong_to_good,
        good_to_wrong,
        to_error,
        with_data_changed;
    };

    track_summary track_table_to_summary(track const &source,
                                         track const &target,
                                         edit_table const &);

    std::ostream &operator<<(std::ostream &os, track_summary const &);

  }

}

#endif
