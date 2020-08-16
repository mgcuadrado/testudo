#ifndef OXYS_TESTIT_TESTSTATS_HEADER_
#define OXYS_TESTIT_TESTSTATS_HEADER_

// test statistics; a "TestStats" object is passed to test functions and later
// processed by formatters to keep track of the number of passed and failed
// test cases

#include <cstdint>

namespace testudo {

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
    constexpr TestStats() : n_passed_p(0), n_failed_p(0), n_errors_p(0) { }
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

    integer const &n_passed=n_passed_p;
    integer const &n_failed=n_failed_p;
    integer const &n_errors=n_errors_p;
  private:
    integer n_passed_p;
    integer n_failed_p;
    integer n_errors_p;
  };

}

#endif
