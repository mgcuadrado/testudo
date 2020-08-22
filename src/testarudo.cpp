#ifndef NAUTOTEST

#include "testudo_lc.h"
#include "testudo_ext.h"

namespace {

  using namespace std;

  define_top_test_node("testudo", testarudo, "testarudo tests", 10);

  define_test(testarudo, simple, "simple testarudo tests") {
    check(true)_true;
  }

  define_test_node(testarudo, complex, "complex testarudo tests");

  define_test(complex, difficult, "difficult test") {
    check(7)_equal(8);
    check("un chat")_equal(string("un chat"));
  }

  define_test(complex, complicated, "complicated test") {
    check("it's")_equal("complicated");
    check(6*9)_equal(42);
  }

  define_test(complex, with_error, "test_with_error") {
    check(22+22)_equal(44);
    perform(throw runtime_error("see if you can catch me!"));
    print_text("this should never appear");
    check("this should never appear either")_equal(string());
  }

  define_test(complex, penultimate, "penultimate test") {
    check("there's nothing")_equal("penultimate");
  }

  define_test_node(testarudo, disorder, "disordered tests");

  define_top_test("testudo.testarudo.disorder",
                  unu, "unu, but comes last", 10) {
    check(string("unu")>string("du"))_true;
  }

  define_top_test("testudo.testarudo.disorder",
                  du, "du, but comes first", 10) {
    check(string("unu")<string("du"))_true;
  }

  with_data_define_top_test(list({2, 4, 5, 6}),
                            "testudo", all_even, "all_even", 20) {
    show_value(datum);
    check(datum%2)_equal(0);
  }

  struct ClassWithNoRepresentation { };
  bool operator==(ClassWithNoRepresentation, ClassWithNoRepresentation)
    { return false; }

  define_top_test("testudo.testarudo.disorder", absdiff, "absdiff()", 80) {
    set_approx_epsilon(1e-10);
    check(testudo::absdiff(5., 7.))_approx(2.);
    check(testudo::absdiff(7., 5.))_approx(2.);
  }

  define_top_test("testudo.testarudo.disorder",
                  no_representation, "class with no representation", 100) {
    check(ClassWithNoRepresentation())_equal(ClassWithNoRepresentation());
  }

  define_top_test("testudo.testarudo.disorder",
                  testarudo_ext, "Testarudo support for STL objects", 90) {
    set_approx_epsilon(1e-6);
    declare(tuple<float, int, double> ta={3.14, 8, 7.5});
    declare(tuple<double, float, int> tb={4.14, 7., 6});
    check(ta)_approx(tb);
    show_value(absdiff(ta, tb));
    declare(tuple<float, int, double> tc={3.14, 8, 7.5});
    check(ta)_approx(tc);
    print_break();
    declare(list<float> la={3.14, 8, 7.5});
    declare(list<double> lb={4.14, 7., 6});
    check(la)_approx(lb);
    show_value(absdiff(la, lb));
    declare(list<double> lc={3.14, 8, 7.5});
    check(la)_approx(lc);
  }

  struct NumbersFixture : testudo::Fixture {
    NumbersFixture(test_parameters)
      : Fixture(test_arguments),
        x(1.), y(-2.5), z(3.14) {
      show_value(x);
      show_value(y);
      show_value(z);
    }
    double x, y, z;
  };

  define_top_test_node("testudo", fixture, "fixture tests", 245);

  define_test(
      fixture,
      addition_commutativity, "+ commutativity",
      with_fixture(NumbersFixture)) {
    check(x+y)_approx(y+x);
  }

  define_test(
      fixture,
      multiplication_commutativity, "* commutativity",
      with_fixture(NumbersFixture)) {
    check(x*y)_approx(y*x);
  }

  define_test(fixture,
              associativity, "associativity",
              with_fixture(NumbersFixture)) {
    check(x*(y+z))_approx(x*y+x*z);
  }

  struct AtDestruction {
    AtDestruction(function<void ()> f) : f(f) { }
    ~AtDestruction() { f(); }
    function<void ()> const f;
  };
  define_top_test("testudo", scope, "scope begin and end", 375) {
    declare(int flag=10);
    { show_scope("at_destruction_flag_20");
      declare(AtDestruction at_destruction_flag_20([&flag]() { flag=20; }));
      { show_scope();
        declare(AtDestruction at_destruction_flag_30([&flag]() { flag=30; }));
        check(flag)_equal(10);
      }
      check(flag)_equal(30);
    }
    check(flag)_equal(20);
  }

}

#endif
