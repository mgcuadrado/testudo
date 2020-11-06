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

#ifndef OXYS_TESTIT_TESTSTATS_HEADER_
#define OXYS_TESTIT_TESTSTATS_HEADER_

// test statistics; a "TestStats" object is passed to test functions and later
// processed by formatters to keep track of the number of passed and failed
// test cases

#include "testudo_macros.h"
#include <cstdint>

namespace testudo___implementation {

  using integer=int_fast64_t;

  // this class just wraps a bool representing wether a single test case
  // succeeded or failed; its purpose is making "TestStats::operator+=()"
  // clearer
  class CheckResult {
  public:
    constexpr CheckResult(bool passed) : passed(passed) { }
    bool const passed;
  };

  // test statistics class; keeps track of passed and failed tests; a
  // "TestStats" object can be incremented with another "TestStats" object (in
  // which case "passed" and "failed" counts are added), or with a
  // "CheckResult" object, which increments either the "passed" counter or the
  // "failed" counter, depending on the value of the "CheckResult" object
  class TestStats {
  public:
    constexpr TestStats(integer n_p, integer n_f, integer n_e)
      : n_passed_p(n_p), n_failed_p(n_f), n_errors_p(n_e) { }
    constexpr TestStats() : TestStats(0, 0, 0) { }
    TestStats &operator+=(TestStats const &arg) {
      n_passed_p+=arg.n_passed_p;
      n_failed_p+=arg.n_failed_p;
      n_errors_p+=arg.n_errors_p;
      return *this;
    }
    TestStats &operator+=(CheckResult arg) {
      ++(arg.passed ? n_passed_p : n_failed_p);
      return *this;
    }
    void unexpected_error() { ++n_errors_p; }

    integer n_passed() const { return n_passed_p; }
    integer n_failed() const { return n_failed_p; }
    integer n_errors() const { return n_errors_p; }
  private:
    integer n_passed_p;
    integer n_failed_p;
    integer n_errors_p;
  };

  inline TestStats operator-(TestStats const &ts1, TestStats const &ts2) {
    return {ts1.n_passed()-ts2.n_passed(),
            ts1.n_failed()-ts2.n_failed(),
            ts1.n_errors()-ts2.n_errors()};
  }

}

testudo___BRING(TestStats)

#endif
