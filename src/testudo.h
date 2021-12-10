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
//     additionally, "testudo_try_catch.h" to use "begintrycatch" and
//       "endtrycatch"

#include "testudo_macros.h"
#include "testudo_format.h"
#include "testudo_base.h"
#include "testudo_activate.h"
#include <string>
#include <map>
#include <list>
#include <memory>
#include <functional>

namespace testudo___implementation {

  constexpr double approx_epsilon_default=1e-6;

  // "push" a test format object: change it upon creation, restore it upon
  // destruction
  struct push_test_format_t {
    push_test_format_t(test_format_p *test_format,
                       test_format_p new_test_format)
      : test_format(test_format), old_test_format(*test_format)
      { *test_format=new_test_format; }
    ~push_test_format_t() { *test_format=old_test_format; }
  private:
    test_format_p *const test_format;
    test_format_p const old_test_format;
  };

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

    // "root_node()"
    static sptr root_node();
    // "get_node()" retrieves a test node by its full name; if it doesn't
    // exist, it, and any missing ancestors, are created as unset nodes
    static sptr get_node(name_t const &full_name);

    using priority_t=unsigned long int;
    static priority_t const priority_default=0;
    using test_f_t=std::function<void (test_management_t)>;

    // make a new node, child to the specified parent
    template <typename... A>
    static sptr make_node(TestFormat::location_t location,
                          sptr parent, name_t name, A &&...a) {
      sptr child=parent->get_child(location, name);
      // whether the child is newly created, or retrieved, if there are
      // additional arguments, use them to set it:
      child->set(std::forward<A>(a)...);
      return child;
    }

    // make a new node, child to the parent specified by its full name
    // ("ancestors")
    template <typename... A>
    static sptr make_node(TestFormat::location_t location,
                          name_t const &ancestors,
                          name_t name, A &&...a) {
      return
        make_node(location, get_node(ancestors), name, std::forward<A>(a)...);
    }

    TestFormat::location_t const location;
    pptr const parent;
    name_t const name, full_name;

    // recursively run tests, depth-first, children first, and print the
    // readout to the supplied stream; if "include" isn't empty, run only those
    // nodes and their descendants; if "glob" isn't empty, run only nodes with
    // matching full names
    TestStats test(test_format_p,
                   std::list<name_t> include={},
                   std::list<std::string> glob={}) const;
    // the ordered list of children is constructed thus: first, the
    // declaration-ordered children, in the order they were declared, then
    // prioritised children, according to their priority (smaller first); if
    // several children have the same priority, alphabetical order
    std::list<sptr> ordered_children() const;
  private:
    // untitled, unprioritised node
    TestNode(TestFormat::location_t, pptr parent, name_t name);

    // get a child by name
    sptr get_child(TestFormat::location_t, name_t name);

    // recursively run tests: run the children's tests in order, depth-first,
    // then own test function if set; if "include" isn't empty, run only those
    // nodes
    void run_tests(test_management_t test_management,
                   std::list<name_t> include,
                   std::list<std::string> glob) const;

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
      set(std::forward<A>(a)...);
      test_f=set_test_f;
    }

    bool matching_name(std::list<std::string> glob) const;
    bool some_matching_function(std::list<std::string> glob) const;

    bool set_title_and_bucket=false;
    title_t title;
    priority_t priority;
    bool set_declaration_order=false;
    test_f_t test_f;
    // all children
    std::map<name_t, sptr> children;
    // order among the children with "declaration_order"
    std::list<name_t> declaration_order_children;
  };

  // please read the file "macros.txt" for a coherent explanation that
  // complements the comments in this file about the design and workings of
  // Testudo macros

#define testudo___LOCATION(file, line)                                  \
  {file, #line}
#define testudo___SET_LOCATION(file, line)                              \
  test_management.format->set_location({file, #line})


#define testudo___TEST_NAME(n) testudo___CAT(testudo____TEST_, n)
#define testudo___TEST_FUNCTION_NAME(n) testudo____TEST_FUNCTION_##n

#define testudo___NAME_TITLE(name, title) name, title

#define testudo___IF_BRACKETS_B(...) B

#define testudo___SELECT_ID_B(line, name_title)                         \
  testudo___TAKE_1ST name_title
#define testudo___SELECT_ID_testudo___IF_BRACKETS_B                     \
  testudo___TEST_NAME_FROM_LINE,
#define testudo___TEST_NAME_FROM_LINE(line, title)                      \
  line

#define testudo___SELECT_KEY_B(line, name_title)                        \
  testudo___STRING(testudo___TAKE_1ST name_title)
#define testudo___SELECT_KEY_testudo___IF_BRACKETS_B                    \
  testudo___TEST_KEY_FROM_TITLE,
#define testudo___TEST_KEY_FROM_TITLE(line, title)                      \
  title

  // ID_NAME_TITLE(12, "title") -> testudo___TEST_12, "title", "title"
  // ID_NAME_TITLE(12, (name, "title")) -> testudo___TEST_name, "name", "title"
#define testudo___ID_KEY_TITLE(line, name_title)                        \
  testudo___EXPAND_ARG1(                                                \
    testudo___CAT(                                                      \
      testudo___SELECT_ID_,                                             \
      testudo___IF_BRACKETS_B name_title))(line, name_title),           \
  testudo___EXPAND_ARG1(                                                \
    testudo___CAT(                                                      \
      testudo___SELECT_KEY_,                                            \
      testudo___IF_BRACKETS_B name_title))(line, name_title),           \
  testudo___SINGLE_OR_2ND(name_title)

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
#define testudo__DEFINE_TOP_TEST_NODE_L(loc, ancestors, ...)            \
  testudo___DEFINE_TEST_NODE_IMPL(                                      \
    testudo___TAKE_2ND loc,                                             \
    ancestors, __VA_ARGS__)
#define testudo__DEFINE_TEST_NODE_L(loc, parent, name_title)            \
  testudo___DEFINE_TEST_NODE_IMPL(                                      \
    testudo___TAKE_2ND loc,                                             \
    testudo___TEST_NAME(parent), name_title,                            \
    testudo___implementation::declaration_order)

#define testudo___DEFINE_TEST_NODE_IMPL(line, l_a_or_p, ...)            \
  testudo___DEFINE_TEST_NODE_IMPL_ID_KEY_TITLE(                         \
    line,                                                               \
    l_a_or_p,                                                           \
    testudo___ID_KEY_TITLE(line, testudo___FIRST(__VA_ARGS__))          \
    testudo___COMMA_REST(__VA_ARGS__))
#define testudo___DEFINE_TEST_NODE_IMPL_ID_KEY_TITLE(...)               \
  testudo___DEFINE_TEST_NODE_IMPL_ID_KEY_TITLE_IMPL(__VA_ARGS__)
#define testudo___DEFINE_TEST_NODE_IMPL_ID_KEY_TITLE_IMPL(              \
    loc, l_a_or_p, id, key, ...)                                        \
  static inline auto testudo___TEST_NAME(id)=                           \
    testudo::TestNode::make_node({}, l_a_or_p, key, __VA_ARGS__)

  struct Fixture {
    Fixture(test_management_t test_management)
      : test_management(test_management) { }
    test_management_t test_management;
    double approx_epsilon=approx_epsilon_default;
  };

  template <typename F=Fixture>
  struct FixtureSpec {
    using fixture_type=F;
    std::string const fixture_name;
    bool const visible=false;
  };

  struct end_mark_t { } constexpr end_mark{};

  template <typename... A>
  auto select_fixture_spec(A &&...) { return FixtureSpec<>{}; }
  template <typename F, typename... A>
  auto select_fixture_spec(FixtureSpec<F> &&fs, A &&...)
    { return std::move(fs); }

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
  decltype(testudo___implementation::select_fixture_spec(__VA_ARGS__))  \
    ::fixture_type

#define testudo__WITH_FIXTURE(...)                                      \
  testudo___implementation::FixtureSpec<__VA_ARGS__>{                   \
    #__VA_ARGS__, false}
#define testudo__VISIBLE_FIXTURE(...)                                   \
  testudo___implementation::FixtureSpec<__VA_ARGS__>{                   \
    #__VA_ARGS__, true}

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
#define testudo__DEFINE_TOP_TEST_L(loc, ancestors, ...)                 \
  testudo___DEFINE_TEST_IMPL(                                           \
    loc,                                                                \
    testudo___TAKE_2ND loc,                                             \
    ancestors, __VA_ARGS__)
#define testudo__DEFINE_TEST_L(loc, parent, ...)                        \
  testudo___DEFINE_TEST_IMPL(                                           \
    loc,                                                                \
    testudo___TAKE_2ND loc,                                             \
    testudo___TEST_NAME(parent),                                        \
    __VA_ARGS__, testudo___implementation::declaration_order)

#define testudo___DEFINE_TEST_IMPL(loc, line, l_a_or_p, ...)            \
  testudo___DEFINE_TEST_ID_KEY_TITLE(                                   \
    loc, line,                                                          \
    l_a_or_p,                                                           \
    testudo___ID_KEY_TITLE(line, testudo___FIRST(__VA_ARGS__))          \
    testudo___COMMA_REST(__VA_ARGS__),                                  \
    testudo___implementation::end_mark)
#define testudo___DEFINE_TEST_ID_KEY_TITLE(...)                         \
  testudo___DEFINE_TEST_ID_KEY_TITLE_IMPL(__VA_ARGS__)
#define testudo___DEFINE_TEST_ID_KEY_TITLE_IMPL(                        \
    loc, line, l_a_or_p, id, key, title, ...)                           \
  /* define a class for the test: */                                    \
  namespace {                                                           \
    struct testudo___TEST_FUNCTION_NAME(id)                             \
      : testudo___FIXTURE_TYPE(__VA_ARGS__) {                           \
      testudo___TEST_FUNCTION_NAME(id)(                                 \
          testudo___implementation::test_management_t test_management,  \
          bool visible)                                                 \
        : testudo___FIXTURE_TYPE(__VA_ARGS__)(                          \
            {(visible                                                   \
              ? test_management.format                                  \
              : testudo___implementation                                \
                    ::make_null_test_format_for_fixtures(               \
                  test_management.format)),                             \
             test_management.test_vfos,                                 \
             test_management.stats}),                                   \
          test_management(test_management),                             \
          visible(visible) { }                                          \
      /* the following hides fixture's version: */                      \
      testudo::test_management_t test_management;                       \
      bool const visible;                                               \
      void perform_test() {                                             \
        if (visible)                                                    \
          test_management.format                                        \
            ->output_text("fixture constructor done");                  \
        do_this_test();                                                 \
        if (visible)                                                    \
          test_management.format                                        \
            ->output_text("fixture destructor");                        \
      }                                                                 \
      void do_this_test(); /* this will contain the test body */        \
    };                                                                  \
  }                                                                     \
  /* define a test node that instantiates the class and calls */        \
  /* "do_this_test()" on it: */                                         \
  static inline auto testudo___TEST_NAME(id)=                           \
    testudo::TestNode::make_node(                                       \
      testudo___LOCATION loc,                                           \
      l_a_or_p, key,                                                    \
      testudo::TestNode::test_f_t(                                      \
        [](testudo::test_management_t test_management) {                \
          auto fixture_spec=select_fixture_spec(__VA_ARGS__);           \
          if (not fixture_spec.fixture_name.empty())                    \
            test_management.format->output_text(                        \
              (fixture_spec.visible                                     \
               ? "visible fixture "                                     \
               : "with fixture ")                                       \
              +fixture_spec.fixture_name);                              \
          testudo___TEST_FUNCTION_NAME(id) fixture(                     \
            test_management, fixture_spec.visible);                     \
          fixture.perform_test();                                       \
        }),                                                             \
      title, testudo___implementation::select_priority(__VA_ARGS__));   \
  /* start the definition of "do_this_test()", but stop just before */  \
  /* the opening "{", so that the user takes it from there */           \
  void testudo___TEST_FUNCTION_NAME(id)::do_this_test()


  void print_tree(std::ostream &, TestNode::csptr node);

  /// test commands

  // convert a value to string using "vfos"
  template <typename T>
  std::string to_text_show_value(value_format_ostream_t test_vfos,
                                 T const &t) {
    std::string result=to_text(test_vfos->fmt_os, t);
    test_vfos->use_up_one_time_manipulators();
    return result;
  }

  // check whether two values are equal, and convert them to text (in the
  // "string &" arguments)
  template <typename ValT, typename RefT>
  bool to_text_equal(value_format_ostream_t test_vfos,
                     ValT const &val, std::string &sval,
                     RefT const &ref, std::string &sref) {
    sval=to_text(test_vfos->fmt_os, val);
    if constexpr (std::is_constructible_v<ValT, RefT>)
      sref=to_text(test_vfos->fmt_os, ValT(ref));
    else
      sref=to_text(test_vfos->fmt_os, ref);
    test_vfos->use_up_one_time_manipulators();
    return are_equal(val, ref);
  }

  // check whether two values are closer than a tolerance
  template <typename T1, typename T2, typename TE>
  bool approx(T1 const &arg1, T2 const &arg2, TE max_error)
    { return (absdiff(arg1, arg2)<=max_error); }

  // check whether two values are closer than a tolerance, and convert them to
  // text (in the "string &" arguments)
  template <typename ValT, typename RefT>
  bool to_text_approx(value_format_ostream_t test_vfos,
                      ValT const &val, std::string &sval,
                      RefT const &ref, std::string &sref,
                      double max_error) {
    sval=to_text(test_vfos->fmt_os, val);
    sref=to_text(test_vfos->fmt_os, ref);
    test_vfos->use_up_one_time_manipulators();
    return approx(val, ref, max_error);
  }

  // check whether a value verifies a predicate and convert it to text (in the
  // "string &" argument)
  template <typename ValT, typename P>
  bool to_text_verify(value_format_ostream_t test_vfos,
                      ValT const &val, std::string &sval,
                      P const &p) {
    sval=to_text(test_vfos->fmt_os, val);
    test_vfos->use_up_one_time_manipulators();
    return p(val);
  }

  // the following is a way to run several instructions while making them
  // appear as a single one, which can be used braceless in a while-loop or
  // if-statement; just surround the instructions in a pair of
  // "testudo___BEGIN_GROUP" and "testudo___END_GROUP"; this generates an
  // anonymous function that is immediately invoked, by grouping the
  // instructions inside "[&]() { ... }()"; this is a better alternative than
  // the classical "do { ... } while (0)"
#define testudo___BEGIN_GROUP [&]() {
#define testudo___END_GROUP           }()

  /// test instruction macros

  // whenever possible, the macro instructions below use "...", so that
  // expressions like "pair<int, int>()" can be handled (if the macro expected
  // one argument only, that argument would be "pair<int" in that case)

  // the following instruction macros assume there is a "TestFormat" called
  // "test_format", and an ostringstream called "test_vfos" that's used for
  // textual representation of values; for the macros "check()_approx()", there
  // must be a double called "approx_epsilon", which will be used in lieu of
  // the argument to "_tol" in "check()_approx()_tol()"

  // we're gonna rename all macros for different styles, so the renamed macros
  // will expand arguments that are macros; in most instances, that's not what
  // we want (if we have a macro "PI" defined as "3.14159...", we want "PI" to
  // appear in the test report, not its definition); therefore, when this
  // matters (i.e., when the original macro would use "#__VA_ARGS__"), the
  // original macro's name will end in "_S", and the macro will accept an "s"
  // argument prior to "...", that will contain the string representation of
  // "...", produced by the renamed macro

#define testudo__STEP_ID(id)                                            \
  test_management.format->output_step_id(#id)

  // print string as is
#define testudo__PRINT_TEXT_L(loc, ...)                                 \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_text(__VA_ARGS__);                   \
  testudo___END_GROUP
  // print a break line
#define testudo__PRINT_BREAK_L(loc)                                     \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_separator();                         \
  testudo___END_GROUP

  struct IndentMarker {
    IndentMarker(test_format_p test_format)
      : test_format(test_format)
      { test_format->output_begin_indent(); }
    ~IndentMarker()
      { test_format->output_end_indent(); }
  private:
    test_format_p const test_format;
  };

  struct AtDestructionBeginIndent {
    AtDestructionBeginIndent(test_format_p test_format)
      : test_format(test_format) { }
    ~AtDestructionBeginIndent()
      { test_format->output_begin_indent(); }
  private:
    test_format_p const test_format;
  };

  struct AtDestructionEndIndent {
    AtDestructionEndIndent(test_format_p test_format)
      : test_format(test_format) { }
    ~AtDestructionEndIndent()
      { test_format->output_end_indent(); }
  private:
    test_format_p const test_format;
  };

  struct ScopeMarker {
    ScopeMarker(test_format_p test_format, std::string name_="")
      : test_format(test_format), name(name_)
      { test_format->output_begin_scope(name); }
    ~ScopeMarker()
      { test_format->output_end_scope(name); }
  private:
    test_format_p const test_format;
    std::string const name;
  };

  struct DeclareScopeMarker {
    DeclareScopeMarker(test_format_p test_format, std::string text)
      : test_format(test_format)
      { test_format->output_begin_declare_scope(text); }
    ~DeclareScopeMarker()
      { test_format->output_end_declare_scope(); }
  private:
    test_format_p const test_format;
  };

  // record a declaration (the argument to "declare()" cannot be enclosed in a
  // "[&]() { ... }()" construct, since the declaration would be done in a
  // deeper scope)
#define testudo__DECLARE_L_S(loc, s, ...)                               \
  testudo___SET_LOCATION loc;                                           \
  test_management.format->output_declare(s); __VA_ARGS__

  // record a performed action
#define testudo__PERFORM_L_S(loc, s, ...)                               \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_perform(s);                          \
    __VA_ARGS__;                                                        \
  testudo___END_GROUP
  // record a declaration, but don't do it (useful in some cases where, for
  // some compilation options, the usual action must be replaced with something
  // else)
#define testudo__FAKE_DECLARE_L_S(loc, s, ...)                          \
  testudo___SET_LOCATION loc;                                           \
  test_management.format->output_declare(s)
  // record an action, but don't do it (useful in some cases where, for some
  // compilation options, the usual action must be replaced with something
  // else)
#define testudo__FAKE_PERFORM_L_S(loc, s, ...)                          \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_perform(s);                          \
  testudo___END_GROUP

  // record a fixture member
#define testudo__FIXTURE_MEMBER_L_S(loc, s, ...)                        \
  testudo___FIXTURE_MEMBER_OUTPUT_L_S(loc, s, __LINE__);                \
  __VA_ARGS__
#define testudo___FIXTURE_MEMBER_OUTPUT_L_S(loc, s, line)               \
  bool testudo___STICK(testudo___done_, line)=                          \
    [this]() {                                                          \
      testudo___SET_LOCATION loc;                                       \
      test_management.format->output_declare("(fixture) " s);           \
      return true;                                                      \
    }()
#define testudo___STICK(a, b) a ## b

  // record a fixture member initialisation
#define testudo__FIXTURE_INIT_L_S(loc, s, v, ...)                       \
  v((testudo___BEGIN_GROUP                                              \
       testudo___SET_LOCATION loc;                                      \
       test_management.format                                           \
         ->output_perform("(fixture) init " #v "(" s ")");              \
     testudo___END_GROUP,                                               \
     __VA_ARGS__))


#define testudo___INSERT_DECLARATION(...) if (__VA_ARGS__; true)
#define testudo___INSERT_ACTION(...) if ((__VA_ARGS__, true))


  template <typename T>
  auto to_testudo_container(std::initializer_list<T> const &il)
    { return std::list<T>(il); }
  template <typename T>
  auto to_testudo_container(T const &t) { return t; }

#define testudo___REMOVE_BRACKETS(...) __VA_ARGS__

#define testudo___ADD_BRACKETS(...) (__VA_ARGS__)
#define testudo___ADD_BRACKETS_IF_NOT_B_B(...) __VA_ARGS__
#define testudo___ADD_BRACKETS_IF_NOT_B_testudo___IF_BRACKETS_B         \
  testudo___ADD_BRACKETS,

#define testudo___ADD_BRACKETS_IF_NEEDED(...)                           \
  testudo___EXPAND_ARG1(                                                \
    testudo___CAT(                                                      \
      testudo___ADD_BRACKETS_IF_NOT_B_,                                 \
      testudo___IF_BRACKETS_B __VA_ARGS__))(__VA_ARGS__)

#define testudo___ADD_SQUARE_BRACKETS(...) [__VA_ARGS__]
#define testudo___NO_SQUARE_BRACKETS_IF_NOT_B_B(...)                    \
  testudo___ADD_SQUARE_BRACKETS __VA_ARGS__
#define testudo___NO_SQUARE_BRACKETS_IF_NOT_B_testudo___IF_BRACKETS_B   \
  testudo___TAKE_SINGLE,

#define testudo___REPLACE_BRACKETS_WITH_SQUARE_BRACKETS(...)            \
  testudo___EXPAND_ARG1(                                                \
    testudo___CAT(                                                      \
      testudo___NO_SQUARE_BRACKETS_IF_NOT_B_,                           \
      testudo___IF_BRACKETS_B __VA_ARGS__))(__VA_ARGS__)


  inline auto to_text_bind(std::ostream &fmt_os) {
    return [&fmt_os](auto const &t) { return to_text(fmt_os, t); };
  }
  inline auto to_text_brackets_bind(std::ostream &fmt_os) {
    return
      [&fmt_os](auto const &...t) { return to_text_brackets(fmt_os, t...); };
  }

#define testudo___TO_TEXT_BRACKETS_IF_B_B to_text_brackets_bind
#define testudo___TO_TEXT_BRACKETS_IF_B_testudo___IF_BRACKETS_B         \
  to_text_bind,
#define testudo___TO_TEXT_TUPLE_IF_BRACKETS(...)                        \
    testudo___EXPAND_ARG1(                                              \
      testudo___CAT(                                                    \
        testudo___TO_TEXT_BRACKETS_IF_B_,                               \
        testudo___IF_BRACKETS_B __VA_ARGS__))

#define testudo__WITH_MULTILINE_DATA_L_S(loc, s, n, ...)                \
  testudo___WITH_DATA_L_S_IMPL(                                         \
    loc, testudo___implementation::break_multiline_data(s), n,          \
    testudo___ADD_BRACKETS_IF_NEEDED(n),                                \
    (testudo___REPLACE_BRACKETS_WITH_SQUARE_BRACKETS(n)),               \
    testudo___implementation::testudo___TO_TEXT_TUPLE_IF_BRACKETS(n),   \
    __VA_ARGS__)
#define testudo__WITH_DATA_L_S(loc, s, n, ...)                          \
  testudo___WITH_DATA_L_S_IMPL(                                         \
    loc, testudo___implementation::break_data(s), n,                    \
    testudo___ADD_BRACKETS_IF_NEEDED(n),                                \
    (testudo___REPLACE_BRACKETS_WITH_SQUARE_BRACKETS(n)),               \
    testudo___implementation::testudo___TO_TEXT_TUPLE_IF_BRACKETS(n),   \
      __VA_ARGS__)
#define testudo___WITH_DATA_L_S_IMPL(                                   \
    loc, s, n, n_b, n_sb, to_text_bind, ...)                            \
  testudo___WITH_DATA_L_S_IMPL_IMPL(                                    \
    loc, s, n, n_b, n_sb, to_text_bind, __VA_ARGS__)
#define testudo___WITH_DATA_L_S_IMPL_IMPL(                              \
    loc, s, n, n_b, n_sb, to_text_bind, ...)                            \
  testudo___INSERT_ACTION(testudo___SET_LOCATION loc)                   \
  testudo___INSERT_DECLARATION(                                         \
    auto testudo___container=                                           \
      testudo___implementation::to_testudo_container(__VA_ARGS__))      \
  if (not testudo___container.empty())                                  \
    testudo___INSERT_DECLARATION(                                       \
      auto testudo___with_log=                                          \
        testudo___implementation::get_with_loop_log(test_management))   \
    testudo___INSERT_DECLARATION(std::size_t testudo___container_i=0)   \
    for (auto const &testudo___REMOVE_BRACKETS n_sb:                    \
           testudo___container)                                         \
      testudo___INSERT_DECLARATION(                                     \
        testudo___implementation::push_test_format_t push_test_format{  \
          &test_management.format,                                      \
          testudo___implementation::make_with_loop_test_format(         \
            test_management.format,                                     \
            testudo___with_log,                                         \
            testudo___container_i==testudo___container.size()-1,        \
            #n, to_text_bind(test_management.test_vfos->fmt_os) n_b,    \
            s)})                                                        \
      testudo___INSERT_ACTION(++testudo___container_i)


  template <typename T=std::exception>
  using default_to_exception_t=T;
  // perform an action that is expected to throw; the thrown exception is
  // recorded; a bool flag tracks whether the exception was thrown, and is
  // checked for trueness with "check()_true()"
#define testudo__CHECK_TRY_L_S(loc, s, ...)                             \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_try(s, false);                       \
    try {                                                               \
      __VA_ARGS__;                                                      \
      test_management.format->output_catch(                             \
        "", "<no exception>",                                           \
        testudo___implementation::false_tag, false);                    \
      test_management.stats+=                                           \
        testudo___implementation::check_result(false);                  \
    }
#define testudo__CATCH_S(s, ...)                                        \
    catch (testudo___implementation                                     \
           ::default_to_exception_t<__VA_ARGS__> const &e) {            \
      test_management.format->output_catch(                             \
        s,                                                              \
        testudo___implementation                                        \
          ::exception_to_text(test_management.test_vfos->fmt_os, e),    \
        testudo___implementation::true_tag, false);                     \
      test_management.stats+=                                           \
        testudo___implementation::check_result(true);                   \
    }                                                                   \
    catch (...) {                                                       \
      test_management.format->output_catch(                             \
        s, "<unexpected exception>",                                    \
        testudo___implementation::false_tag, false);                    \
      test_management.stats+=                                           \
        testudo___implementation::check_result(false);                  \
      throw;                                                            \
    }                                                                   \
  testudo___END_GROUP

  // show the value of an expression
#define testudo__SHOW_VALUE_L_S(loc, s, ...)                            \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_show_value(                          \
      s,                                                                \
      testudo___implementation::to_text_show_value(                     \
        test_management.test_vfos,__VA_ARGS__));                        \
  testudo___END_GROUP

  // print "begin scope", and automatically print "end scope" at the end of the
  // current scope; if an argument is given, it's used as a tag in the "begin
  // scope" and "end scope" messages
#define testudo__IN_SCOPE_L(loc, ...)                                   \
  testudo___INSERT_ACTION(testudo___SET_LOCATION loc)                   \
  testudo___INSERT_DECLARATION(                                         \
    testudo___implementation::ScopeMarker testudo___scope_marker(       \
      test_management.format, std::string(__VA_ARGS__)))

  // record a scoped declaration
#define testudo__WITH_DECLARE_L_S(loc, s, ...)                          \
  testudo___INSERT_ACTION(testudo___SET_LOCATION loc)                   \
  testudo___INSERT_DECLARATION(                                         \
    testudo___implementation::DeclareScopeMarker                        \
      testudo___declare_scope_marker(                                   \
        test_management.format, s))                                     \
  testudo___INSERT_DECLARATION(__VA_ARGS__)


  // define the tolerance for "check()_approx()" (to be used only in functions
  // not declared with "define_test()" and similar macros; for instance, in an
  // external function that is called from a "define_test()" function)
#define testudo__DEFINE_APPROX_EPSILON_L(loc, ...)                      \
  testudo__DECLARE_L_S(loc,                                             \
                       "double approx_epsilon="#__VA_ARGS__,            \
                     double approx_epsilon=__VA_ARGS__)
  // set the tolerance for "check()_approx()"
#define testudo__SET_APPROX_EPSILON_L(loc, ...)                         \
  testudo__PERFORM_L_S(loc,                                             \
                       "approx_epsilon="#__VA_ARGS__,                   \
                       approx_epsilon=__VA_ARGS__)
  // show the tolerance for "check()_approx()"
#define testudo__SHOW_APPROX_EPSILON_L(loc)                             \
  testudo__SHOW_VALUE_L_S(loc, "approx_epsilon", approx_epsilon)

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

  // the support for "provided(expression) { ... }" is similar, but takes the
  // form of an "if" statement to be followed by the provided-protected code

  // hold a value as a copy if it's an rvalue, or else as a reference
  template <typename T>
  class HoldValue {
  public:
    HoldValue(T &&t) // rvalue: hold a copy
      : value_p(std::make_unique<T>(std::move(t))), value(*value_p) { }
    HoldValue(T const &t) // otherwise, point a reference
      : value(t) { }
  private:
    std::unique_ptr<T const> value_p;
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

  using truth_f=std::function<bool (bool)>;
  inline truth_f const affirmation=[](bool b) { return b; };
  inline truth_f const negation=[](bool b) { return not b; };
  inline truth_f const always_false=[](bool) { return false; };
  using check_result_to_text_f=std::function<std::string (bool)>;
  inline check_result_to_text_f const true_or_false=
    [](bool b) { return b ? true_tag : false_tag; };
  inline check_result_to_text_f const true_or_error=
    [](bool b) { return b ? true_tag : error_tag; };

  // special class to hold the argument to "check()"
  template <typename A>
  struct Check {
    test_management_t const test_management;
    bool const is_a_valid;
    HoldValue<A> const a;
    std::string const sa;
    check_result_to_text_f const check_result_to_text;

    template <typename AA>
    Check(test_management_t test_management, AA &&a, std::string sa,
          bool error_if_fail=false)
      : test_management(test_management),
        is_a_valid(is_valid(a)), a(std::forward<AA>(a)), sa(sa),
        check_result_to_text(error_if_fail ? true_or_error : true_or_false) { }

    template <typename Cont>
    auto check_cont(Cont &&cont) const {
      return cont.check(*this, is_a_valid ? affirmation : always_false, "");
    }
    template <typename Cont>
    auto check_cont_inverse(Cont &&cont) const {
      return cont.check(*this, is_a_valid ? negation : always_false, "nay");
    }

    template <typename P>
    Check &set(std::string name, P const &p) {
      check_params[name]=p;
      return *this;
    }
    template <typename P>
    P get(std::string name, P const &p_default) const {
      if (auto it=check_params.find(name); it==check_params.end())
        return p_default;
      else
        return std::any_cast<P>(it->second);
    }
  private:
    std::map<std::string, std::any> check_params;
  };
  // we need to construct "Check<A>" indirectly: this function detects type "A"
  // (which can't be deduced using "decltype()", since it doesn't work well
  // with immediate lambda evaluations); the constructor of "Check<A>" is again
  // templatised, to have a universal reference on the "A" parameter
  template <typename A>
  auto make_check(test_management_t tm, A &&a, std::string sa)
    { return Check<std::decay_t<A>>(tm, std::forward<A>(a), sa); }
  template <typename A>
  auto make_provided(test_management_t tm, A &&a, std::string sa)
    { return Check<std::decay_t<A>>(tm, std::forward<A>(a), sa, true); }

  struct inverse_t { } constexpr inverse{};

  // class to check for trueness
  struct CheckTrue {
    CheckTrue() { }

    template <typename A>
    bool check(Check<A> const &c, truth_f tf, std::string prefix) {
      bool result=tf(c.a.value);
      std::string text_result=c.check_result_to_text(result);
      c.test_management.format
        ->output_check_true(c.sa, text_result, prefix, false);
      c.test_management.stats+=check_result(text_result);
      return result;
    }
  };
  // the reason to have a function to construct "CheckTrue()" is for
  // homogeneity with the other checking classes below
  inline auto make_check_true() { return CheckTrue(); }

  template <typename... T>
  std::string to_text_multiple(value_format_ostream_t test_vfos,
                               T &&...t)
    { return to_text_comma_separated(test_vfos->fmt_os, t...); }

  // class to check for trueness, or else, showing given values
  struct CheckTrueFor {
    value_format_ostream_t const test_vfos;
    bool const are_v_valid;
    std::string valv, sv;

    template <typename... V>
    CheckTrueFor(value_format_ostream_t test_vfos,
                 std::string sv, V &&...v)
      : test_vfos(test_vfos),
        are_v_valid((... and is_valid(v))),
        valv(to_text_multiple(test_vfos, std::forward<V>(v)...)), sv(sv) { }

    template <typename A>
    bool check(Check<A> const &c, truth_f tf, std::string prefix) {
      bool result=are_v_valid and tf(c.a.value);
      std::string text_result=c.check_result_to_text(result);
      c.test_management.format
        ->output_check_true_for(c.sa, sv, valv, text_result, prefix, false);
      c.test_management.stats+=check_result(text_result);
      return result;
    }
  };
  // see "Check" for the reason to have a function to construct this templated
  // class, rather than construct it directly
  template <typename... V>
  auto make_check_true_for(value_format_ostream_t test_vfos,
                           std::string sv, V &&...v)
    { return CheckTrueFor(test_vfos, sv, std::forward<V>(v)...); }

  // class to check for equality
  template <typename B>
  struct CheckEqual {
    value_format_ostream_t const test_vfos;
    bool const is_b_valid;
    HoldValue<B> const b;
    std::string const sb;

    template <typename BB>
    CheckEqual(value_format_ostream_t test_vfos,
               BB &&b, std::string sb)
      : test_vfos(test_vfos),
        is_b_valid(is_valid(b)), b(std::forward<BB>(b)), sb(sb) { }

    template <typename A>
    bool check(Check<A> const &c, truth_f tf, std::string prefix) {
      std::string vala, valb;
      bool result=
        tf(to_text_equal(test_vfos, c.a.value, vala, b.value, valb));
      std::string text_result=c.check_result_to_text(result);
      c.test_management.format->output_check_equal(
        c.sa, vala, sb, valb, text_result, prefix, false);
      c.test_management.stats+=check_result(text_result);
      return result;
    }
  };
  // see "Check" for the reason to have a function to construct this templated
  // class, rather than construct it directly
  template <typename B>
  auto make_check_equal(value_format_ostream_t test_vfos,
                        B &&b, std::string sb) {
    return CheckEqual<std::decay_t<B>>(test_vfos,
                                       std::forward<B>(b), sb);
  }

  // special class to check for nearness; on destruction,
  // "CheckApprox<B>::OnDestruction<A>" checks the two expressions have
  // approximately equal values, up to "approx_epsilon" (shown as "eps" on the
  // output), or to a specified tolerance
  template <typename B>
  struct CheckApprox {
    value_format_ostream_t const test_vfos;
    bool const is_b_valid;
    HoldValue<B> const b;
    std::string const sb;
    double const default_tol;
    std::string const default_stol;

    template <typename BB>
    CheckApprox(value_format_ostream_t test_vfos,
                BB &&b, std::string sb,
                double default_tol, std::string default_stol)
      : test_vfos(test_vfos),
        is_b_valid(is_valid(b)), b(std::forward<BB>(b)), sb(sb),
        default_tol(default_tol), default_stol(default_stol) { }

    template <typename A>
    bool check(Check<A> const &c, truth_f tf, std::string prefix) {
      std::string vala, valb;
      double tol=c.get("tol", default_tol);
      std::string stol=c.get("stol", default_stol);
      bool result=
        tf(to_text_approx(test_vfos,c.a.value, vala, b.value, valb, tol));
      std::string text_result=c.check_result_to_text(result);
      c.test_management.format->output_check_approx(
        c.sa, vala, sb, valb, stol, text_result, prefix, false);
      c.test_management.stats+=check_result(text_result);
      return result;
    }
  };
  // see "Check" for the reason to have a function to construct this templated
  // class, rather than construct it directly
  template <typename B>
  auto make_check_approx(value_format_ostream_t test_vfos,
                         B &&b, std::string sb, double tol, std::string stol) {
    return CheckApprox<std::decay_t<B>>(test_vfos,
                                        std::forward<B>(b), sb, tol, stol);
  }

  // class to check for predicate-verification
  template <typename B>
  struct CheckVerify {
    value_format_ostream_t const test_vfos;
    bool const is_b_valid;
    HoldValue<B> const b;
    std::string const sb;

    template <typename BB>
    CheckVerify(value_format_ostream_t test_vfos,
                BB &&b, std::string sb)
      : test_vfos(test_vfos),
        is_b_valid(is_valid(b)), b(std::forward<BB>(b)), sb(sb) { }

    template <typename A>
    bool check(Check<A> const &c, truth_f tf, std::string prefix) {
      std::string vala;
      bool result=
        tf(to_text_verify(test_vfos, c.a.value, vala, b.value));
      std::string text_result=c.check_result_to_text(result);
      c.test_management.format->output_check_verify(
        c.sa, vala, sb, text_result, prefix, false);
      c.test_management.stats+=check_result(text_result);
      return result;
    }
  };
  // see "Check" for the reason to have a function to construct this templated
  // class, rather than construct it directly
  template <typename B>
  auto make_check_verify(value_format_ostream_t test_vfos,
                         B &&b, std::string sb) {
    return CheckVerify<std::decay_t<B>>(test_vfos,
                                        std::forward<B>(b), sb);
  }

  template <typename T>
  struct testarudo_decay {
    using type=T;
  };
  template <>
  struct testarudo_decay<char const *> : testarudo_decay<std::string> { };
  template <>
  struct testarudo_decay<char *> : testarudo_decay<std::string> { };
  template <typename T>
  using testarudo_decay_t=typename testarudo_decay<std::decay_t<T>>::type;

#define testudo__CHECK_L_S(loc, s, ...)                                 \
  (testudo___BEGIN_GROUP                                                \
     testudo___SET_LOCATION loc;                                        \
     using testudo___check_type [[maybe_unused]]=                       \
       testudo___implementation::testarudo_decay_t<                     \
         decltype(__VA_ARGS__)>;                                        \
     testudo___implementation::make_check(                              \
      test_management, __VA_ARGS__, s).

#define testudo__PROVIDED_L_S(loc, s, ...)                              \
  if (testudo___implementation::AtDestructionEndIndent                  \
        testudo___at_destruction_end_indent(test_management.format);    \
      testudo___BEGIN_GROUP                                             \
        testudo___SET_LOCATION loc;                                     \
        using testudo___check_type [[maybe_unused]]=                    \
          testudo___implementation::testarudo_decay_t<                  \
            decltype(__VA_ARGS__)>;                                     \
        testudo___implementation::AtDestructionBeginIndent              \
        testudo___at_destruction_begin_indent(test_management.format);  \
        return testudo___implementation::make_provided(                 \
          test_management, __VA_ARGS__, s).

#define testudo__EQUAL_S(s, ...)                                        \
    check_cont(testudo___implementation::make_check_equal(              \
                 test_management.test_vfos,                             \
                 testudo___check_type(__VA_ARGS__), s));                \
  testudo___END_GROUP)
#define testudo__NOT_EQUAL_S(s, ...)                                    \
    check_cont_inverse(testudo___implementation::make_check_equal(      \
                         test_management.test_vfos,                     \
                         testudo___check_type(__VA_ARGS__), s));        \
  testudo___END_GROUP)

#define testudo__TOL_S(s, ...)                                          \
  set("tol", double(__VA_ARGS__)).set("stol", std::string(s)).
#define testudo__APPROX_S(s, ...)                                       \
    check_cont(testudo___implementation::make_check_approx(             \
                 test_management.test_vfos,                             \
                 testudo___check_type(__VA_ARGS__), s,                  \
                 approx_epsilon, "eps"));                               \
  testudo___END_GROUP)
#define testudo__NOT_APPROX_S(s, ...)                                   \
    check_cont_inverse(testudo___implementation::make_check_approx(     \
                         test_management.test_vfos,                     \
                         testudo___check_type(__VA_ARGS__), s,          \
                         approx_epsilon, "eps"));                       \
  testudo___END_GROUP)

#define testudo__TRUE                                                   \
    check_cont(testudo___implementation::make_check_true());            \
  testudo___END_GROUP)
#define testudo__FALSE                                                  \
    check_cont_inverse(testudo___implementation::make_check_true());    \
  testudo___END_GROUP)

#define testudo__TRUE_FOR_S(s, ...)                                     \
    check_cont(testudo___implementation::make_check_true_for(           \
                 test_management.test_vfos,                             \
                 s, __VA_ARGS__));                                      \
  testudo___END_GROUP)

#define testudo__FALSE_FOR_S(s, ...)                                    \
    check_cont_inverse(testudo___implementation::make_check_true_for(   \
                         test_management.test_vfos,                     \
                         s, __VA_ARGS__));                              \
  testudo___END_GROUP)

#define testudo__VERIFY_S(s, ...)                                       \
    check_cont(testudo___implementation::make_check_verify(             \
                 test_management.test_vfos,                             \
                 (__VA_ARGS__), s));                                    \
  testudo___END_GROUP)
#define testudo__NOT_VERIFY_S(s, ...)                                   \
    check_cont_inverse(testudo___implementation::make_check_verify(     \
                         test_management.test_vfos,                     \
                         (__VA_ARGS__), s));                            \
  testudo___END_GROUP)

  /// predicates
  template <typename L>
  class predicate_t {
  public:
    predicate_t(L const &l) : l(l) { }
    template <typename T>
    bool operator()(T const &t) const { return l(t); }
  private:
    L const l;
  };

  template <typename L>
  auto operator not(predicate_t<L> const &p) {
    return predicate_t([p](auto const &v)
                         { return not p(v); });
  }

  template <typename L1, typename L2>
  auto operator and(predicate_t<L1> const &p1, predicate_t<L2> const &p2) {
    return predicate_t([p1, p2](auto const &v)
                         { return p1(v) and p2(v); });
  }

  template <typename L1, typename L2>
  auto operator or(predicate_t<L1> const &p1, predicate_t<L2> const &p2) {
    return predicate_t([p1, p2](auto const &v)
                         { return p1(v) or p2(v); });
  }

#define testudo__PREDICATE testudo___implementation::predicate_t
#define testudo__PREDICATE_A(...)                                       \
  testudo__PREDICATE_C_A((), __VA_ARGS__)
#define testudo__PREDICATE_C_A(b_c, ...)                                \
  testudo__PREDICATE([testudo___REMOVE_BRACKETS b_c](auto const &a)     \
                       { return (__VA_ARGS__); })

  template <typename F>
  auto generate_data(std::size_t size, F const &f) {
    std::list<decltype(f())> result;
    for (std::size_t i=0; i<size; ++i)
      result.emplace_back(f());
    return result;
  }

  template <typename... F>
  auto generate_data_tuple(std::size_t size, F const &...f) {
    std::list<std::tuple<decltype(f())...>> result;
    for (std::size_t i=0; i<size; ++i)
      result.emplace_back(f()...);
    return result;
  }

  template <typename T>
  auto cartesian_product(std::list<T> const &tl) {
    std::list<std::tuple<T>> result;
    for (auto const &t: tl)
      result.emplace_back(t);
    return result;
  }

  template <typename T, typename U, typename... V>
  auto cartesian_product(std::list<T> const &tl,
                         std::list<U> const &ul,
                         std::list<V> const &...vl) {
    auto cpuvl=cartesian_product(ul, vl...);
    std::list<std::tuple<T, U, V...>> result;
    for (auto const &t: tl)
      for (auto const &uv: cpuvl)
        result.emplace_back(std::tuple_cat(std::tuple{t}, uv));
    return result;
  }

#define testudo__TFOS                                                   \
  test_management.test_vfos->fmt_os

}


testudo___BRING(TestNode,
                print_tree,
                Fixture,
                generate_data, generate_data_tuple, cartesian_product)

#endif
