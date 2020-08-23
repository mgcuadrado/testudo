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

#ifndef MGCUADRADO_TESTUDO_HEADER_
#define MGCUADRADO_TESTUDO_HEADER_

// what to #include:
//
//     "testudo.h" to write the tests
//
//     "testudo_base.h" to merely specialise "absdiff()" or "to_text()"
//
//     "testudo_ext.h" instead of "testudo_base.h" if your test expressions use
//       STL containers (see "testudo_ext.h" to check which containers are
//       supported)
//
//     additionally, "testudo_try_catch.h" to use "begintrycatch" and
//       "endtrycatch"

#include "testudo_format.h"
#include "testudo_base.h"
#include <string>
#include <map>
#include <list>
#include <memory>
#include <functional>

namespace testudo {

  constexpr double approx_epsilon_default=1e-6;

#define testudo__TEST_PARAMETERS                                        \
  [[maybe_unused]] testudo::test_format_p test_format,                  \
  [[maybe_unused]] testudo::TestStats &test_stats
#define testudo__TEST_ARGUMENTS test_format, test_stats

  /// test structure

  // "TestNode" and "Test" aren't meant to be used directly by the user;
  // instead, they're used through the macros below

  using name_t=std::string;
  using title_t=std::string;
  struct declaration_order_t { } constexpr declaration_order;

  /*
        .--------------------------.
        | TestNode                 |<------------------.
        |--------------------------|  *                |
        | name, full_name, title   |                   |
        | bucket                   |                   |
        | test_f                   |                   |
        |--------------------------|                   |
        | make_test_node<>()       |<x>----------------'
        | test()                   |     map<priority_t,
        | run_tests()              |         list>
        '--------------------------'
  */

  // "TestNode" is a node in the test tree; it can be a mere node, with a name
  // and title, or a test, with a name, title, and test function; a "TestNode"
  // must be created with "TestNode::make_test_node<>()", where the template
  // argument is the concrete test node type, and defaults to "TestNode"; the
  // return value is a shared pointer to the newly created test node; the
  // arguments are a parent specification, the name, and the title; the parent
  // specification is a shared pointer to the parent; there can be an
  // additional argument, stating the child bucket, that defaults either to
  // bucketless or to priority 0; when available, "bucketless" is realised by
  // specifying "declaration_order"; the order of the chidren is determined by
  // the growing value of their bucket priority, then, within a bucket, by name
  // order; nevertheless, the order among bucketless children (i.e., children
  // for whose creation the shared pointer to the parent was used, which get a
  // bucket priority of -1) is the declaration order (i.e., the order in the
  // translation unit); if a function is given to the constructor, it specifies
  // the function to be run by the test node.
  class TestNode {
  public:
    using sptr=std::shared_ptr<TestNode>; // shared pointer
    using csptr=std::shared_ptr<TestNode const>; // shared pointer to const
    using pptr=TestNode *; // pointer to parent

    // "root_node()" (a singleton) is the root node of the test tree
    static sptr root_node();
    // "get_node()" retrieves a test node by its full name; if it doesn't
    // exist, it, and any missing ancestors, are created as unset nodes
    static sptr get_node(std::string const &full_name);

    using priority_t=unsigned long int;
    static priority_t const priority_default=0;
    using test_f_t=std::function<void (testudo__TEST_PARAMETERS)>;

    // make a new node, child to the specified parent
    template <typename... A>
    static sptr make_node(sptr parent, name_t name, A &&...a) {
      sptr child=parent->get_child(name);
      // whether the child is newly created, or retrieved, if there are
      // additional arguments, use them to set it:
      child->set(a...);
      return child;
    }

    // make a new node, child to the parent specified by its full name
    // ("ancestors")
    template <typename... A>
    static sptr make_node(std::string const &ancestors,
                          name_t name, A &&...a)
      { return make_node(get_node(ancestors), name, a...); }

    pptr const parent;
    name_t const name, full_name;

    // recursively run tests, depth-first, children first, and print the
    // readout to the supplied stream; if "subtree" isn't empty, run only
    // that subtree
    testudo::TestStats test(testudo::test_format_p,
                            std::string subtree="") const;
    // the ordered list of children is constructed thus: first, the
    // declaration-ordered children, in the order they were declared, then
    // prioritised children, according to their priority (smaller first); if
    // several children have the same priority, alphabetical order
    std::list<sptr> ordered_children() const;
  private:
    // untitled, unprioritised node
    TestNode(pptr parent, name_t name);

    // get a child by name
    sptr get_child(name_t name);

    // recursively run tests: run the children's tests in order, depth-first,
    // then own test function if set
    void run_tests(testudo::test_format_p, testudo::TestStats &) const;

    // null setting
    void set() { }
    // throw if already set; otherwise, set
    void check_and_set();
    // title and priority (defaults to 0)
    void set(title_t set_title, priority_t set_priority=priority_default);
    // title and declaration-order specification
    void set(title_t set_title, declaration_order_t);
    // test function and possibly other things
    template <typename... A>
    void set(test_f_t set_test_f, A &&...a) {
      set (a...);
      test_f=set_test_f;
    }

    bool set_title_and_bucket=false;
    title_t title;
    priority_t priority;
    bool set_declaration_order=false;
    test_f_t test_f;
    // all children
    std::map<std::string, sptr> children;
    // order among the children with "declaration_order"
    std::list<std::string> declaration_order_children;
  };

  // please read the file "macros.txt" for a coherent explanation that
  // complements the comments in this file about the design and workings of
  // Testudo macros

#define testudo___DECL_EAT_A_SEMICOLON                                  \
  struct never_define_this_struct_68e47e1831e0eebbe5994ad57527f71b

#define testudo___TEST_NAME(n) testudo____TEST_##n
#define testudo___TEST_FUNCTION_NAME(n) testudo____TEST_FUNCTION_##n

#define testudo__TOP_TEST_NODE(full_name)                               \
  testudo::TestNode::get_node(full_name)

  // a plain test node (i.e., and test node with no test function) is defined
  // with
  //
  //     define_test_node(parent_shared_pointer,
  //                      test_node_name,
  //                      "test node description");
  //
  // this creates a shared pointer to the test node, called "test_node_name",
  // ordered among its siblings using the specified bucket priority if any, or
  // in declaration order otherwise; if the test node is a top test node (i.e.,
  // the top test node defined in a module, not the root test node defined for
  // the whole library), the macro is "define_top_test_node()", and the default
  // bucket priority is 0
#define testudo__DEFINE_TOP_TEST_NODE(ancestors, name, ...)             \
  testudo___DEFINE_TEST_NODE_IMPL(ancestors, name, __VA_ARGS__)
#define testudo__DEFINE_TEST_NODE(parent, name, title)                  \
  testudo___DEFINE_TEST_NODE_IMPL(                                      \
    testudo___TEST_NAME(parent), name,                                  \
    title, testudo::declaration_order)

#define testudo___DEFINE_TEST_NODE_IMPL(l_a_or_p, name, ...)            \
  static inline auto testudo___TEST_NAME(name)=                         \
    testudo::TestNode::make_node(l_a_or_p, #name, __VA_ARGS__)          \


  struct Fixture {
    Fixture(testudo__TEST_PARAMETERS)
      : test_format(test_format), test_stats(test_stats) { }
    testudo::test_format_p const test_format;
    testudo::TestStats &test_stats;
    double approx_epsilon=approx_epsilon_default;
  };

  template <typename F=Fixture>
  struct FixtureSpec {
    using fixture_type=F;
    std::string const fixture_name;
  };

  struct end_mark_t { } constexpr end_mark{};

  template <typename... A>
  auto select_fixture_spec(A &&...) { return FixtureSpec<>{}; }
  template <typename F, typename... A>
  auto select_fixture_spec(FixtureSpec<F> &&fs, A &&...) { return fs; }

  inline auto select_priority(end_mark_t)
    { return TestNode::priority_default; }
  template <typename B>
  auto select_priority(B b, end_mark_t)
    { return b; }
  template <typename F>
  auto select_priority(FixtureSpec<F>, end_mark_t e)
    { return select_priority(e); }
  template <typename F, typename B>
  auto select_priority(FixtureSpec<F>, B b, end_mark_t)
    { return b; }

#define testudo___FIXTURE_TYPE(...)                                     \
  decltype(testudo::select_fixture_spec(__VA_ARGS__))::fixture_type

#define testudo__WITH_FIXTURE(...)                                      \
  testudo::FixtureSpec<__VA_ARGS__>{#__VA_ARGS__}

  // a test (i.e., a test node with a test function) is defined with
  //
  //     define_test(parent_shared_pointer,
  //                 test_name,
  //                 "test description,
  //                 [with_fixture(FixtureTypeName)]) { // this is optional
  //       ... // test instructions
  //     }
  //
  // this defines a class that derives from the fixture, and a member in the
  // class, called "do_this_test()", with the function body as written after
  // the "define_test()" macro, and a shared pointer to the test, called
  // "test_name"; a top test is defined with "define_top_test()" instead; the
  // bucket priority specification is analogous to "define_test_node()" and
  // "define_top_test_node()"
#define testudo__DEFINE_TOP_TEST(ancestors, name, ...)                  \
  testudo___DEFINE_TEST_IMPL(ancestors, name, __VA_ARGS__)
#define testudo__DEFINE_TEST(parent, name, ...)                         \
  testudo___DEFINE_TEST_IMPL(                                           \
    testudo___TEST_NAME(parent), name,                                  \
    __VA_ARGS__, testudo::declaration_order)

#define testudo___DEFINE_TEST_IMPL(...)                                 \
  testudo___DEFINE_TEST_IMPL_IMPL(__VA_ARGS__, testudo::end_mark)
#define testudo___DEFINE_TEST_IMPL_IMPL(l_a_or_p, name, title, ...) \
  /* define a class for the test: */                                    \
  struct testudo___TEST_FUNCTION_NAME(name)                             \
    : testudo___FIXTURE_TYPE(__VA_ARGS__) {                             \
    testudo___TEST_FUNCTION_NAME(name)(testudo__TEST_PARAMETERS)        \
      : testudo___FIXTURE_TYPE(__VA_ARGS__)(                            \
          testudo__TEST_ARGUMENTS) { }                                  \
    void do_this_test(); /* this will contain the test body */          \
  };                                                                    \
  /* define a test node that instantiates the class and calls */        \
  /* "do_this_test()" on it: */                                         \
  static inline auto testudo___TEST_NAME(name)=                         \
    testudo::TestNode::make_node(                                       \
      l_a_or_p, #name,                                                  \
      testudo::TestNode::test_f_t([](testudo__TEST_PARAMETERS) {        \
        auto fixture_spec=select_fixture_spec(__VA_ARGS__);             \
        if (not fixture_spec.fixture_name.empty())                      \
          test_format->output_text(                                     \
            "with fixture "+fixture_spec.fixture_name);                 \
        testudo___TEST_FUNCTION_NAME(name)(                             \
          testudo__TEST_ARGUMENTS).do_this_test();                      \
      }),                                                               \
      title, testudo::select_priority(__VA_ARGS__));                    \
  /* start the definition of "do_this_test()", but stop just before */  \
  /* the opening "{", so that the user takes it from there */           \
  void testudo___TEST_FUNCTION_NAME(name)::do_this_test()

  template <typename DataT, typename DoTestDatum>
  void do_test_data(DoTestDatum &&do_test_datum, DataT const &data) {
    unsigned long int datum_number=0;
    for (auto const &datum: data) {
      ++datum_number;
      do_test_datum.test_format
        ->output_show_value("datum_number", std::to_string(datum_number));
      do_test_datum.do_this_test(datum);
      do_test_datum.test_format->output_separator();
    }
  }

#define testudo__WITH_DATA_DEFINE_TOP_TEST(data, ancestors, name, ...)  \
  testudo___WITH_DATA_DEFINE_TEST_IMPL(                                 \
    data, ancestors, name, __VA_ARGS__)
#define testudo__WITH_DATA_DEFINE_TEST(data, parent, name, ...)         \
  testudo___WITH_DATA_DEFINE_TEST_IMPL(                                 \
    data, testudo___TEST_NAME(parent), name,                            \
    __VA_ARGS__, testudo::declaration_order)
#define testudo___WITH_DATA_DEFINE_TEST_IMPL(data, parent, name, ...)   \
  template <typename T>                                                 \
  struct testudo___TEST_FUNCTION_NAME(name) : testudo::Fixture {        \
    testudo___TEST_FUNCTION_NAME(name)(testudo__TEST_PARAMETERS)        \
      : Fixture(testudo__TEST_ARGUMENTS),                               \
      test_format(test_format), test_stats(test_stats) { }              \
    void do_this_test(T const &);                                       \
    testudo::test_format_p const test_format;                           \
    testudo::TestStats &test_stats;                                     \
  };                                                                    \
  static inline auto testudo___TEST_NAME(name)=                         \
    testudo::TestNode::make_node(                                       \
      parent, #name,                                                    \
      testudo::TestNode::test_f_t([](testudo__TEST_PARAMETERS) {        \
        using T=std::decay_t<decltype(*data.begin())>;                  \
        using Test=testudo___TEST_FUNCTION_NAME(name)<T>;   \
        do_test_data(Test(testudo__TEST_ARGUMENTS), data);              \
      }),                                                               \
      __VA_ARGS__);                                                     \
  template <typename T>                                                 \
  void testudo___TEST_FUNCTION_NAME(name)<T>                            \
    ::do_this_test(T const &datum)

  void print_tree(std::ostream &, testudo::TestNode::csptr node);

  /// test commands

  // check whether two values are equal, and convert them to text (in the
  // "string &" arguments)
  template <typename RefT, typename ValT>
  bool to_text_equal(RefT const &ref, std::string &sref,
                     ValT const &val, std::string &sval) {
    sref=to_text(ref);
    if constexpr (std::is_constructible_v<RefT, ValT>)
      sval=to_text(RefT(val));
    else
      sval=to_text(val);
    return ref==val;
  }
  // specialise for the common case where the reference is a "char const *",
  // converting it to a string
  template <typename ValT>
  bool to_text_equal(char const *ref, std::string &sref,
                     ValT const &val, std::string &sval)
    { return to_text_equal(std::string(ref), sref, val, sval); }

  // check whether two values are closer than a tolerance
  template <typename T1, typename T2, typename TE>
  bool approx(T1 const &arg1, T2 const &arg2, TE max_error)
    { return (absdiff(arg1, arg2)<=max_error); }

  // check whether two values are closer than a tolerance, and convert them to
  // text (in the "string &" arguments)
  template <typename RefT, typename ValT>
  bool to_text_approx(RefT const &ref, std::string &sref,
                      ValT const &val, std::string &sval,
                      double max_error) {
    sref=to_text(ref);
    sval=to_text(val);
    return approx(val, ref, max_error);
  }

  /// test instruction macros

  // whenever possible, the macro instructions below use "...", so that
  // expressions like "pair<int, int>()" can be handled (if the macro expected
  // one argument only, that argument would be "pair<int" in that case)

  // the following instruction macros assume there is a "TestFormat" called
  // "test_format"; for the macros "check()_approx()", there must be a double
  // called "approx_epsilon", which will be used in lieu of the argument to
  // "_tol" in "check()_approx()_tol()"

  // we're gonna rename all macros for different styles, so the renamed macros
  // will expand arguments that are macros; in most instances, that's not what
  // we want (if we have a macro "PI" defined as "3.14159...", we want "PI" to
  // appear in the test report, not its definition); therefore, when this
  // matters (i.e., when the original macro would use "#__VA_ARGS__"), the
  // original macro's name will end in "_S", and the macro will accept an "s"
  // argument prior to "...", that will contain the string representation of
  // "...", produced by the renamed macro

  // print string as is
#define testudo__PRINT_TEXT(...)                                        \
  test_format->output_text(__VA_ARGS__)
  // print possibly multiline string as is
#define testudo__PRINT_MULTILINE_TEXT(...)                              \
  test_format->output_multiline_text(__VA_ARGS__)
  // print a break line
#define testudo__PRINT_BREAK()                                          \
  test_format->output_separator()

  struct ScopeMarker {
    ScopeMarker(test_format_p test_format, std::string name_="")
      : test_format(test_format), name(name_.empty() ? "<unnamed>" : name_)
      { test_format->output_begin_scope(name); }
    ~ScopeMarker()
      { test_format->output_end_scope(name); }
  private:
    test_format_p const test_format;
    std::string const name;
  };

  // print "begin scope", and automatically print "end scope" at the end of the
  // current scope; if an argument is given, it's used as a tag in the "begin
  // scope" and "end scope" messages
#define testudo__SHOW_SCOPE(...)                                        \
  testudo::ScopeMarker testudo_scope_marker(                            \
    test_format, std::string(__VA_ARGS__))

  // record a declaration (the argument to "declare()" cannot be enclosed in a
  // "do { ... } while (0)" construct, since the declaration would be done in a
  // deeper scope)
#define testudo__DECLARE_S(s, ...)                                      \
  test_format->output_declare(s); __VA_ARGS__

  // record a performed action
#define testudo__PERFORM_S(s, ...)                                      \
  do {                                                                  \
    test_format->output_perform(s);                                     \
    __VA_ARGS__;                                                        \
  } while (0)
  // record a declaration, but don't do it (useful in some cases where, for
  // some compilation options, the usual action must be replaced with something
#define testudo__FAKE_DECLARE_S(s, ...)                                 \
  test_format->output_declare(s)
  // else) record an action, but don't do it (useful in some cases where, for
  // some compilation options, the usual action must be replaced with something
  // else)
#define testudo__FAKE_PERFORM(s, ...)                                   \
  test_format->output_perform(s)

  // perform an action that is expected to throw; the thrown exception is
  // recorded; the "flag" argument names a bool flag that tracks whether the
  // exception was thrown, and is checked for trueness with "check()_true()"
#define testudo__CHECK_TRY_CATCH_S(flag, s, ...)                        \
  do {                                                                  \
    bool flag=false;                                                    \
    test_format->output_try(s, #flag);                                  \
    try { __VA_ARGS__; }                                                \
    catch (std::exception const &e)                                     \
      { flag=true; test_format->output_catch(e.what()); }               \
    testudo__CHECK_S(#flag, flag) testudo__TRUE;                        \
  } while (0)

  // show the value of an expression
#define testudo__SHOW_VALUE_S(s, ...)                                   \
  test_format->output_show_value(s, testudo::to_text(__VA_ARGS__))

  // the the possibly multiline value of an expression
#define testudo__SHOW_MULTILINE_VALUE_S(s, ...)                         \
  test_format->output_show_multiline_value(                             \
    s, testudo::to_text(__VA_ARGS__))

  // define the tolerance for "check()_approx()" (to be used only in functions
  // not declared with "define_test()" and similar macros; for instance, in an
  // external function that is called from a "define_test()" function)
#define testudo__DEFINE_APPROX_EPSILON(...)                             \
  testudo__DECLARE_S("double approx_epsilon="#__VA_ARGS__,              \
                     double approx_epsilon=__VA_ARGS__)
  // set the tolerance for "check()_approx()"
#define testudo__SET_APPROX_EPSILON(...)                                \
  testudo__PERFORM_S("approx_epsilon="#__VA_ARGS__,                     \
                     approx_epsilon=__VA_ARGS__)
  // show the tolerance for "check()_approx()"
#define testudo__SHOW_APPROX_EPSILON()                                  \
  testudo__SHOW_VALUE_S("approx_epsilon", approx_epsilon)

  /// multi-macro checking instructions

  // a checking instruction starts with "check(expression)", which stores the
  // expression and various pieces of information into a special object; the
  // expansion of "check()" ends with a dot so that a second macro is needed;
  // the second macro invokes a method of the first special object, turning it
  // into another kind of special object that holds more information related to
  // the arguments to the second macro, if any; additional macros can be
  // chained depending on the previous ones; all this does is constructing a
  // special object that holds all the information to perform the check; since
  // the special object isn't used in any way, it is immediately destroyed; the
  // actual check is done in the object's destructor

  // hold a value as a copy if it's an rvalue, or else as a reference
  template <typename T>
  class HoldValue {
  public:
    HoldValue(T &&t) // rvalue: hold a copy
      : value_p(std::make_shared<T>(std::move(t))), value(*value_p) { }
    HoldValue(T const &t) // otherwise, point a reference
      : value(t) { }
  private:
    std::shared_ptr<T const> value_p;
  public:
    T const &value;
  };

  // specializations for various kinds of "char *"
  template <>
  class HoldValue<char const *> {
  public:
    HoldValue(char const *t) : value(t) { }
    std::string value;
  };
  template <>
  class HoldValue<char *> : public HoldValue<char const *>
    { using HoldValue<char const *>::HoldValue; };
  template <std::size_t n>
  class HoldValue<char const [n]> : public HoldValue<char const *>
    { using HoldValue<char const *>::HoldValue; };
  template <std::size_t n>
  class HoldValue<char [n]> : public HoldValue<char const *>
    { using HoldValue<char const *>::HoldValue; };

  // special class to hold the argument to "check()"
  template <typename A>
  struct Check {
    testudo::test_format_p const test_format;
    testudo::TestStats &test_stats;
    HoldValue<A> const a;
    std::string const sa;

    template <typename AA>
    Check(testudo__TEST_PARAMETERS, AA &&a, std::string sa)
      : test_format(test_format), test_stats(test_stats),
        a(std::forward<AA>(a)), sa(sa) { }

    template <typename Cont>
    auto check_cont(Cont &&cont) const { return cont.check(*this); }

  };
  // we need to construct "Check<A>" indirectly: this function detects type "A"
  // (which can't be deduced using "decltype()", since it doesn't work well
  // with immediate lambda evaluations); the constructor of "Check<A>" is again
  // templatised, to have a universal reference on the "A" parameter
  template <typename A>
  auto make_check(testudo__TEST_PARAMETERS, A &&a, std::string sa) {
    return Check<std::decay_t<A>>(testudo__TEST_ARGUMENTS,
                                  std::forward<A>(a), sa);
  }

  // class to check for trueness
  struct CheckTrue {
    template <typename A>
    void check(Check<A> const &c) {
      bool check_result=c.a.value;
      c.test_format->output_check_true(c.sa, check_result);
      c.test_stats+=testudo::CheckResult(check_result);
    }
  };
  // the reason to have a function to construct "CheckTrue()" is for
  // homogeneity with the other checking classes below
  inline auto make_check_true() { return CheckTrue(); }

  // class to check for equality
  template <typename B>
  struct CheckEqual {
    HoldValue<B> const b;
    std::string const sb;

    template <typename BB>
    CheckEqual(BB &&b, std::string sb)
      : b(std::forward<BB>(b)), sb(sb) { }

    template <typename A>
    void check(Check<A> const &c) {
      std::string vala, valb;
      bool check_result=
        testudo::to_text_equal(c.a.value, vala, b.value, valb);
      c.test_format->output_check_equal(c.sa, vala, sb, valb, check_result);
      c.test_stats+=testudo::CheckResult(check_result);
    }
  };
  // see "Check" for the reason to have a function to construct this templated
  // class, rather than construct it directly
  template <typename B>
  auto make_check_equal(B &&b, std::string sb)
    { return CheckEqual<std::decay_t<B>>(std::forward<B>(b), sb); }

  // special class to check for nearness; on destruction,
  // "CheckApprox<B>::OnDestruction<A>" checks the two expressions have
  // approximately equal values, up to "approx_epsilon" (shown as "eps" on the
  // output), or to a specified tolerance
  template <typename B>
  struct CheckApprox {
    HoldValue<B> const b;
    std::string const sb;
    double tol;
    std::string stol;

    template <typename BB>
    CheckApprox(BB &&b, std::string sb,
                double tol, std::string stol)
      : b(std::forward<BB>(b)), sb(sb), tol(tol), stol(stol) { }

    template <typename A>
    struct OnDestruction {
      OnDestruction(Check<A> const &c, CheckApprox<B> const &r)
        : c(c), r(r) { }
      void set_tol(double new_tol, std::string new_stol)
        { r.tol=new_tol; r.stol=new_stol; }
      ~OnDestruction() {
        std::string vala, valb;
        bool check_result=
          testudo::to_text_approx(c.a.value, vala, r.b.value, valb, r.tol);
        c.test_format->output_check_approx(c.sa, vala, r.sb, valb, r.stol,
                                           check_result);
        c.test_stats+=testudo::CheckResult(check_result);
      }
      Check<A> const &c;
      CheckApprox<B> r; // can't be kept by ref (disappears in a smaller scope)
    };

    template <typename A>
    OnDestruction<A> check(Check<A> const &r) { return {r, *this}; }
  };
  // see "Check" for the reason to have a function to construct this templated
  // class, rather than construct it directly
  template <typename B>
  auto make_check_approx(B &&b, std::string sb, double tol, std::string stol)
    { return CheckApprox<std::decay_t<B>>(std::forward<B>(b), sb, tol, stol); }

#define testudo__CHECK_S(s, ...)                                        \
  testudo::make_check(testudo__TEST_ARGUMENTS, (__VA_ARGS__), s).


#define testudo__EQUAL_S(s, ...)                                        \
  check_cont(testudo::make_check_equal((__VA_ARGS__), s))

#define testudo__APPROX_S(s, ...)                                       \
  check_cont(testudo::make_check_approx((__VA_ARGS__), s, \
                                        approx_epsilon, "eps"))

#define testudo__TOL_S(s, ...)                                          \
  .set_tol((__VA_ARGS__), s)

#define testudo__TRUE                                                   \
  check_cont(testudo::make_check_true())

}

#endif
