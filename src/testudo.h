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
//     "testudo_ext.h" instead of "testudo.h" if your test expressions use STL
//       containers (see "testudo_ext.h" to check which containers are
//       supported)
//
//     additionally, "testudo_try_catch.h" to use "begintrycatch" and
//       "endtrycatch"

#include "testudo_macros.h"
#include "testudo_format.h"
#include "testudo_base.h"
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

    // "root_node()" (a singleton) is the root node of the test tree
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
      child->set(a...);
      return child;
    }

    // make a new node, child to the parent specified by its full name
    // ("ancestors")
    template <typename... A>
    static sptr make_node(TestFormat::location_t location,
                          name_t const &ancestors,
                          name_t name, A &&...a)
      { return make_node(location, get_node(ancestors), name, a...); }

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
      set (a...);
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

#define testudo___DECL_EAT_A_SEMICOLON                                  \
  struct never_define_this_struct_68e47e1831e0eebbe5994ad57527f71b

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

  // check whether two values are equal, and convert them to text (in the
  // "string &" arguments)
  template <typename ValT, typename RefT>
  bool to_text_equal(ValT const &val, std::string &sval,
                     RefT const &ref, std::string &sref) {
    sval=to_text(val);
    if constexpr (std::is_constructible_v<ValT, RefT>)
      sref=to_text(ValT(ref));
    else
      sref=to_text(ref);
    return are_equal(val, ref);
  }

  // check whether two values are closer than a tolerance
  template <typename T1, typename T2, typename TE>
  bool approx(T1 const &arg1, T2 const &arg2, TE max_error)
    { return (absdiff(arg1, arg2)<=max_error); }

  // check whether a value verifies a predicate
  template <typename ValT, typename P>
  bool to_text_verify(ValT const &val, std::string &sval,
                      P const &p) {
    sval=to_text(val);
    return p(val);
  }

  // check whether two values are closer than a tolerance, and convert them to
  // text (in the "string &" arguments)
  template <typename ValT, typename RefT>
  bool to_text_approx(ValT const &val, std::string &sval,
                      RefT const &ref, std::string &sref,
                      double max_error) {
    sval=to_text(val);
    sref=to_text(ref);
    return approx(val, ref, max_error);
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

#define testudo__STEP_ID(id)                                            \
  test_management.format->output_step_id(#id)

  // print string as is
#define testudo__PRINT_TEXT_L(loc, ...)                                 \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_text(__VA_ARGS__);                   \
  testudo___END_GROUP
  // print possibly multiline string as is
#define testudo__PRINT_MULTILINE_TEXT_L(loc, ...)                       \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_multiline_text(__VA_ARGS__);         \
  testudo___END_GROUP
  // print a break line
#define testudo__PRINT_BREAK_L(loc)                                     \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_separator();                         \
  testudo___END_GROUP

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
#define testudo__FAKE_PERFORM(s, ...)                                   \
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

  template <typename... T>
  std::string to_text_brackets(T const &... t) {
    std::list<std::string> tts;
    (tts.push_back(to_text(t)), ...);
    std::string result;
    bool first=true;
    for (auto t: tts) {
      result+=(first ? "" : ", ")+t;
      first=false;
    }
    return "("+result+")";
  }

#define testudo___TO_TEXT_BRACKETS_IF_B_B to_text_brackets
#define testudo___TO_TEXT_BRACKETS_IF_B_testudo___IF_BRACKETS_B         \
  to_text,
#define testudo___TO_TEXT_TUPLE_IF_BRACKETS(...)                        \
  testudo___EXPAND_ARG1(                                                \
    testudo___CAT(                                                      \
      testudo___TO_TEXT_BRACKETS_IF_B_,                                 \
      testudo___IF_BRACKETS_B __VA_ARGS__))

#define testudo__WITH_MULTILINE_DATA_L_S(loc, s, n, ...)                \
  testudo___WITH_DATA_L_S_IMPL(                                         \
    loc, testudo___implementation::break_multiline_data(s), n,          \
    testudo___ADD_BRACKETS_IF_NEEDED(n),                                \
    (testudo___REPLACE_BRACKETS_WITH_SQUARE_BRACKETS(n)),               \
    testudo___TO_TEXT_TUPLE_IF_BRACKETS(n),                             \
    __VA_ARGS__)
#define testudo__WITH_DATA_L_S(loc, s, n, ...)                          \
  testudo___WITH_DATA_L_S_IMPL(                                         \
    loc, testudo___implementation::break_data(s), n,                    \
    testudo___ADD_BRACKETS_IF_NEEDED(n),                                \
    (testudo___REPLACE_BRACKETS_WITH_SQUARE_BRACKETS(n)),               \
    testudo___TO_TEXT_TUPLE_IF_BRACKETS(n),                             \
    __VA_ARGS__)
#define testudo___WITH_DATA_L_S_IMPL(                                   \
    loc, s, n, n_b, n_sb, to_text, ...)                                 \
  testudo___WITH_DATA_L_S_IMPL_IMPL(                                    \
    loc, s, n, n_b, n_sb, to_text, __VA_ARGS__)
#define testudo___WITH_DATA_L_S_IMPL_IMPL(                              \
    loc, s, n, n_b, n_sb, to_text, ...)                                 \
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
            #n, testudo___implementation::to_text n_b,                  \
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
        "", "<no exception>", "false", false);                          \
      test_management.stats+=                                           \
        testudo___implementation::CheckResult(false);                   \
    }
#define testudo__CATCH_S(s, ...)                                        \
    catch (testudo___implementation                                     \
           ::default_to_exception_t<__VA_ARGS__> const &e) {            \
      test_management.format->output_catch(                             \
        s, testudo___implementation::exception_to_text(e),              \
        "true", false);                                                 \
      test_management.stats+=                                           \
        testudo___implementation::CheckResult(true);                    \
    }                                                                   \
    catch (...) {                                                       \
      test_management.format->output_catch(                             \
        s, "<unexpected exception>", "false", false);                   \
      test_management.stats+=                                           \
        testudo___implementation::CheckResult(false);                   \
      throw;                                                            \
    }                                                                   \
  testudo___END_GROUP

  // show the value of an expression
#define testudo__SHOW_VALUE_L_S(loc, s, ...)                            \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_show_value(                          \
      s, testudo::to_text(__VA_ARGS__));                                \
  testudo___END_GROUP

  // the the possibly multiline value of an expression
#define testudo__SHOW_MULTILINE_VALUE_L_S(loc, s, ...)                  \
  testudo___BEGIN_GROUP                                                 \
    testudo___SET_LOCATION loc;                                         \
    test_management.format->output_show_multiline_value(                \
      s, testudo::to_text(__VA_ARGS__));                                \
  testudo___END_GROUP

  // print "begin scope", and automatically print "end scope" at the end of the
  // current scope; if an argument is given, it's used as a tag in the "begin
  // scope" and "end scope" messages
#define testudo__IN_SCOPE_L(loc, ...)                                   \
  testudo___INSERT_ACTION(testudo___SET_LOCATION loc)                   \
  testudo___INSERT_DECLARATION(                                         \
    testudo___implementation::ScopeMarker testudo_scope_marker(         \
      test_management.format, std::string(__VA_ARGS__)))

  // record a scoped declaration
#define testudo__WITH_DECLARE_L_S(loc, s, ...)                          \
  testudo___INSERT_ACTION(testudo___SET_LOCATION loc)                   \
  testudo___INSERT_DECLARATION(                                         \
    testudo___implementation::DeclareScopeMarker                        \
      testudo_declare_scope_marker(                                     \
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

  using truth_f=std::function<bool (bool)>;
  truth_f const truth=[](bool b) { return b; };
  truth_f const falsehood=[](bool b) { return not b; };
  truth_f const always_false=[](bool) { return false; };

  // special class to hold the argument to "check()"
  template <typename A>
  struct Check {
    test_management_t const test_management;
    HoldValue<A> const a;
    bool const is_a_valid;
    std::string const sa;

    template <typename AA>
    Check(test_management_t test_management, AA &&a, std::string sa)
      : test_management(test_management),
        a(std::forward<AA>(a)), is_a_valid(is_valid(a)), sa(sa) { }

    template <typename Cont>
    auto check_cont(Cont &&cont) const
      { return cont.check(*this, is_a_valid ? truth : always_false, ""); }
    template <typename Cont>
    auto check_cont_inverse(Cont &&cont) const {
      return cont.check(*this, is_a_valid ? falsehood : always_false, "nay");
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
  auto make_check(test_management_t test_management, A &&a, std::string sa)
    { return Check<std::decay_t<A>>(test_management, std::forward<A>(a), sa); }

  struct inverse_t { } constexpr inverse{};

  // class to check for trueness
  struct CheckTrue {
    CheckTrue() { }

    template <typename A>
    void check(Check<A> const &c, truth_f tf, std::string prefix) {
      bool check_result=tf(c.a.value);
      c.test_management.format
        ->output_check_true(c.sa, to_text(check_result), prefix, false);
      c.test_management.stats+=CheckResult(check_result);
    }
  };
  // the reason to have a function to construct "CheckTrue()" is for
  // homogeneity with the other checking classes below
  inline auto make_check_true() { return CheckTrue(); }

  inline std::string to_text_multiple()
    { return ""; }
  template <typename A>
  std::string to_text_multiple(A &&a)
    { return testudo::to_text(a); }
  template <typename F, typename... R>
  std::string to_text_multiple(F &&f, R &&... r)
    { return to_text_multiple(f)+", "+to_text_multiple(r...); }

  // class to check for trueness, or else, showing given values
  struct CheckTrueFor {
    std::string valv, sv;

    template <typename... V>
    CheckTrueFor(std::string sv, V &&...v)
      : valv(to_text_multiple(v...)), sv(sv) { }

    template <typename A>
    void check(Check<A> const &c, truth_f tf, std::string prefix) {
      bool check_result=tf(c.a.value);
      c.test_management.format
        ->output_check_true_for(c.sa, sv, valv, to_text(check_result),
                                prefix, false);
      c.test_management.stats+=CheckResult(check_result);
    }
  };
  // see "Check" for the reason to have a function to construct this templated
  // class, rather than construct it directly
  template <typename... V>
  auto make_check_true_for(std::string sv, V &&...v)
    { return CheckTrueFor(sv, std::forward<V>(v)...); }

  // class to check for equality
  template <typename B>
  struct CheckEqual {
    HoldValue<B> const b;
    std::string const sb;

    template <typename BB>
    CheckEqual(BB &&b, std::string sb)
      : b(std::forward<BB>(b)), sb(sb) { }

    template <typename A>
    void check(Check<A> const &c, truth_f tf, std::string prefix) {
      std::string vala, valb;
      bool check_result=
        tf(to_text_equal(c.a.value, vala, b.value, valb));
      c.test_management.format->output_check_equal(
        c.sa, vala, sb, valb, to_text(check_result), prefix, false);
      c.test_management.stats+=CheckResult(check_result);
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
    double const default_tol;
    std::string const default_stol;

    template <typename BB>
    CheckApprox(BB &&b, std::string sb,
                double default_tol, std::string default_stol)
      : b(std::forward<BB>(b)), sb(sb),
        default_tol(default_tol), default_stol(default_stol) { }

    template <typename A>
    void check(Check<A> const &c, truth_f tf, std::string prefix) {
      std::string vala, valb;
      double tol=c.get("tol", default_tol);
      std::string stol=c.get("stol", default_stol);
      bool check_result=
        tf(to_text_approx(c.a.value, vala, b.value, valb, tol));
      c.test_management.format->output_check_approx(
        c.sa, vala, sb, valb, stol, to_text(check_result), prefix, false);
      c.test_management.stats+=CheckResult(check_result);
    }
  };
  // see "Check" for the reason to have a function to construct this templated
  // class, rather than construct it directly
  template <typename B>
  auto make_check_approx(B &&b, std::string sb, double tol, std::string stol)
    { return CheckApprox<std::decay_t<B>>(std::forward<B>(b), sb, tol, stol); }

  // class to check for predicate-verification
  template <typename B>
  struct CheckVerify {
    HoldValue<B> const b;
    std::string const sb;

    template <typename BB>
    CheckVerify(BB &&b, std::string sb)
      : b(std::forward<BB>(b)), sb(sb) { }

    template <typename A>
    void check(Check<A> const &c, truth_f tf, std::string prefix) {
      std::string vala;
      bool check_result=tf(to_text_verify(c.a.value, vala, b.value));
      c.test_management.format->output_check_verify(
        c.sa, vala, sb, to_text(check_result), prefix, false);
      c.test_management.stats+=CheckResult(check_result);
    }
  };
  // see "Check" for the reason to have a function to construct this templated
  // class, rather than construct it directly
  template <typename B>
  auto make_check_verify(B &&b, std::string sb)
    { return CheckVerify<std::decay_t<B>>(std::forward<B>(b), sb); }

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
  {                                                                     \
    testudo___SET_LOCATION loc;                                         \
    auto testudo___check_value=(__VA_ARGS__);                           \
    using testudo___check_type [[maybe_unused]]=                        \
      testudo___implementation::testarudo_decay_t<                      \
        decltype(testudo___check_value)>;                               \
    testudo___implementation::make_check(                               \
      test_management, testudo___check_value, s).


#define testudo__EQUAL_S(s, ...)                                        \
    check_cont(testudo___implementation::make_check_equal(              \
                 testudo___check_type(__VA_ARGS__), s));                \
  } testudo___DECL_EAT_A_SEMICOLON
#define testudo__NOT_EQUAL_S(s, ...)                                    \
    check_cont_inverse(testudo___implementation::make_check_equal(      \
                         testudo___check_type(__VA_ARGS__), s));        \
  } testudo___DECL_EAT_A_SEMICOLON

#define testudo__TOL_S(s, ...)                                          \
  set("tol", double(__VA_ARGS__)).set("stol", std::string(s)).
#define testudo__APPROX_S(s, ...)                                       \
    check_cont(testudo___implementation::make_check_approx(             \
                 testudo___check_type(__VA_ARGS__), s,                  \
                 approx_epsilon, "eps"));                               \
  } testudo___DECL_EAT_A_SEMICOLON
#define testudo__NOT_APPROX_S(s, ...)                                   \
    check_cont_inverse(testudo___implementation::make_check_approx(     \
                         testudo___check_type(__VA_ARGS__), s,          \
                         approx_epsilon, "eps"));                       \
  } testudo___DECL_EAT_A_SEMICOLON

#define testudo__TRUE                                                   \
    check_cont(testudo___implementation::make_check_true());            \
  } testudo___DECL_EAT_A_SEMICOLON
#define testudo__FALSE                                                  \
    check_cont_inverse(testudo___implementation::make_check_true());    \
  } testudo___DECL_EAT_A_SEMICOLON

#define testudo__TRUE_FOR_S(s, ...)                                     \
    check_cont(testudo___implementation::make_check_true_for(           \
                 s, __VA_ARGS__));                                      \
  } testudo___DECL_EAT_A_SEMICOLON

#define testudo__FALSE_FOR_S(s, ...)                                    \
    check_cont_inverse(testudo___implementation::make_check_true_for(   \
                 s, __VA_ARGS__));                                      \
  } testudo___DECL_EAT_A_SEMICOLON

#define testudo__VERIFY_S(s, ...)                                       \
    check_cont(testudo___implementation::make_check_verify(             \
                 (__VA_ARGS__), s));                                    \
  } testudo___DECL_EAT_A_SEMICOLON
#define testudo__NOT_VERIFY_S(s, ...)                                   \
    check_cont_inverse(testudo___implementation::make_check_verify(     \
                         (__VA_ARGS__), s));                            \
  } testudo___DECL_EAT_A_SEMICOLON

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

}


testudo___BRING(TestNode,
                print_tree,
                Fixture,
                generate_data)

#endif
