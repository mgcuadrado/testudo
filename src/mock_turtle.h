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

#ifndef MGCUADRADO_TESTUDO_MOCK_TURTLE_HEADER_
#define MGCUADRADO_TESTUDO_MOCK_TURTLE_HEADER_

#include "testudo_macros.h"
#include "testudo_ext.h"
#include "testudo.h"
#include <tuple>
#include <unordered_set>

// include auto-generated macros; matching defines here are commented-out
#include "mock_turtle_macro_n.h"

namespace testudo___implementation {

  // return values are encoded as a tuple of 0 (for void return types) or 1
  // (for non-void return types) elements
  template <typename T>
  struct ret_tuple {
    using type=std::tuple<T>;
  };
  template <>
  struct ret_tuple<void> {
    using type=std::tuple<>;
  };
  template <typename T>
  using ret_tuple_t=typename ret_tuple<T>::type;

  template <typename T>
  T get_ret(std::tuple<T> const &r) { return std::get<0>(r); }
  inline void get_ret(std::tuple<> const &) { }

  template <typename T>
  auto make_ret(T const &r) { return std::make_tuple(r); }
  inline auto make_ret() { return std::make_tuple(); }

  template <typename T>
  std::string ret_to_text(std::tuple<T> const &r)
    { return to_text(get_ret(r)); }
  inline std::string ret_to_text(std::tuple<> const &) { return "void"; }


  // get info about a signature
  template <typename S>
  struct signature_info;
  template <typename R, typename... A>
  struct signature_info<R (A...)> {
    using ret_type=ret_tuple_t<R>;
    using args_type=std::tuple<A...>;
    template <std::size_t i>
    using arg_type=std::tuple_element_t<i-1, args_type>; // 1-based
  };
  // tuple-encoded return type
  template <typename S>
  using ret_t=typename signature_info<S>::ret_type;
  // tuple-encoded args type
  template <typename S>
  using args_t=typename signature_info<S>::args_type;
  // a specific arg type (1-based)
  template <typename S, std::size_t i>
  using arg_t=typename signature_info<S>::arg_type<i>;
  // tuple-encoded return and args types, in a pair
  template <typename S>
  using ret_args_t=std::pair<ret_t<S>, args_t<S>>;

  // a call record contains information to identify a call: the method name,
  // the return value, and the value of the arguments; it can be generated in
  // one of two ways; first, by retrieving it from a call ledger, with an
  // expected method name and signature; in this case, if the method name
  // doesn't match the logged method name, or if the call ledger doesn't match
  // the logged call ledger, an invalid call record is created; second, from
  // the arguments in the second macro of a "check" macro construct; in this
  // case, the braced-expression constructor is automatically used, and the
  // required parameters are the return value as a tuple and the value of the
  // arguments as a tuple; the comparison rules are such tha thet "check" macro
  // can only be successful if both call records are valid, thus rendering any
  // comparison with a non-matching method name false
  template <typename S>
  class call_record {
    using content_t=ret_args_t<S>;
  public:
    // braced-expression constructor (for "check()"-et-al macros); the other
    // constructors have a number of arguments different from two, to avoid any
    // confusion with this constructor
    call_record(ret_t<S> const &r, args_t<S> const &a)
      : method_name(), is_valid_p(true),
        content(std::make_shared<content_t>(r, a)) { }
    // constructor for a call record with a mismatching method name (invalid)
    call_record(std::string method_name)
      : method_name(method_name), is_valid_p(false), content() { }
    // constructor for a call record with a matching method name
    call_record(std::string method_name,
                content_t const &content, bool is_valid)
      : method_name(method_name), is_valid_p(is_valid),
        content(std::make_shared<content_t>(content)) { }

    auto ret() const { return check_valid(), get_ret(content->first); }
    auto args() const { return check_valid(), content->second; }

    std::string content_to_text() const {
      return
        "{"+ret_to_text(content->first)
        +" (" +testudo___implementation::to_text_get(content->second)+")}";
    }
    std::string const method_name;
    bool is_valid() const { return is_valid_p; }
  private:
    bool const is_valid_p;
    void check_valid() {
      if (not is_valid())
        throw std::runtime_error("invalid call record");
    }
  public:
    std::shared_ptr<content_t const> const content;
  };

  template <typename S>
  bool is_valid_testudo(call_record<S> const &cr) { return cr.is_valid(); }

  template <typename S>
  bool operator==(call_record<S> const &c1, call_record<S> const &c2)
    { return c1.is_valid() and c2.is_valid() and (*c1.content==*c2.content); }

  template <typename S>
  double absdiff_testudo(call_record<S> const &c1, call_record<S> const &c2) {
    if (c1.is_valid() and c2.is_valid())
      return absdiff(*c1.content, *c2.content);
    else
      return 0.;
  }

  template <typename S>
  std::string to_text(call_record<S> const &c) {
    if (c.is_valid()) { // "method_name:{ret (arg1, arg2)}"
      return
        c.method_name+(c.method_name.empty() ? "" : ":")+c.content_to_text();
    }
    else // "method_name:invalid"
      return c.method_name+":invalid";
  }


  template <typename E>
  struct throw_exception {
    throw_exception(E const &exception) : exception(exception) { }
    E const exception;
  };

  ret_tuple_t<void> constexpr void_v{};

  // return value schedule
  template <typename S>
  struct Schedule {
    using R=ret_t<S>;
    Schedule() : default_value(convert(R())) { }
    template <typename T>
    Schedule(T const &t) : default_value(convert(t)) { }
    template <typename T>
    void set_default(T const &t) { default_value=convert(t); }
    R return_value() {
      if (ret_values_p.empty()) // when the schedule empties, use default value
        return default_value();
      else {
        auto r=ret_values_p.front()();
        ret_values_p.pop_front();
        return r;
      }
    }
    template <typename... T>
    void push_back(T const &...t)
      { (ret_values_p.push_back(convert(t)), ...); }
  private:
    template <typename T>
    static auto convert(T const &t,
                        std::enable_if_t<std::is_convertible_v<T, R>, int> =0)
      { return convert(R(t)); }
    static auto convert(R const &r) { return [r]() { return r; }; }
    template <typename E>
    static auto convert(throw_exception<E> const &te)
      { return [te]() -> R { throw te.exception; }; }
    template <typename F>
    static auto convert(F const &f,
                        std::enable_if_t<std::is_convertible_v<
                        decltype(std::declval<F>()()), R>, int> =0)
      { return f; }
    std::function<R ()> default_value;
    std::list<std::function<R ()>> ret_values_p;
  };

  // per-method log of return value and value of the arguments
  template <typename S>
  struct RetArgsLog {
    auto ret_values() const {
      std::vector<ret_t<S>> result;
      std::transform(ret_args_values_p.begin(), ret_args_values_p.end(),
                     std::back_inserter(result),
                     [](auto p) { return p.first; });
      return result;
    }
    auto arg_values() const {
      std::vector<args_t<S>> result;
      std::transform(ret_args_values_p.begin(), ret_args_values_p.end(),
                     std::back_inserter(result),
                     [](auto p) { return p.second; });
      return result;
    }
    auto ret_args_values() const { return ret_args_values_p; }
    auto size() const { return ret_args_values_p.size(); }
    template <typename... T>
    ret_t<S> &log_values(args_t<S> const &a) {
      return
        ret_args_values_p.insert(ret_args_values_p.end(), {ret_t<S>(), a})
          ->first;
    }
  private:
    std::vector<ret_args_t<S>> ret_args_values_p;
  };

  template <typename R, typename... A>
  auto get_return_value_and_log(Schedule<R (A...)> &s, RetArgsLog<R (A...)> &l,
                                std::tuple<A...> const &tu) {
    auto &r=l.log_values(tu);
    r=s.return_value();
    return get_ret(r);
  }
  template <typename T=void>
  class MockClass;

  struct MockClassEntry {
    MockClass<> const *call_ledger;
    std::string mock_name;
    std::string method_name;
    std::size_t method_call_n;
  };
  inline std::string to_text(MockClassEntry const &cle) {
    return
      "{"+cle.mock_name+(cle.mock_name.empty() ? "" : ".")
      +cle.method_name+" @ "+to_text(cle.method_call_n)+"}";
  }

  template <>
  class MockClass<void> {
  public:
    void report_to(MockClass<> *parent, std::string as)
      { testudo___parents.emplace_back(parent, as); }
    void log_call(std::string method_name, std::size_t method_call_n) const
      { testudo___log_call(this, "", method_name, method_call_n); }
    auto const &calls() const { return testudo___calls_p; }
    template <typename M>
    struct iterate_t {
      iterate_t(M const *mock) : mock(mock) { }
      M const *mock;
      bool done() const { return (testudo___index>=mock->calls().size()); }
      operator bool() const { return not done(); }
      void reset() { testudo___index=0; }
      void next(std::size_t steps=1) { testudo___index+=steps; }

      std::string mock_name() const
        { return done() ? "" : mock->calls()[testudo___index].mock_name; }
      std::string method_name() const
        { return done() ? "" : mock->calls()[testudo___index].method_name; }
      template <typename S>
      call_record<S> get_call_implementation(MockClass<> const *call_ledger,
                                             std::string exp_method_name,
                                             RetArgsLog<S> &log) const {
        if (done())
          return {exp_method_name};
        auto const &call=mock->calls()[testudo___index];
        if ((call.call_ledger==call_ledger)
            and (call.method_name==exp_method_name))
          return {exp_method_name,
                  log.ret_args_values()[call.method_call_n],
                  true};
        else
          return {exp_method_name};
      }
      template <typename S>
      call_record<S> pop_call_implementation(MockClass<> const *call_ledger,
                                             std::string exp_method_name,
                                             RetArgsLog<S> &log) {
        auto result=get_call_implementation(call_ledger, exp_method_name, log);
        if (result.is_valid())
          next();
        return result;
      }
    private:
      std::size_t testudo___index=0;
    };
  private:
    void testudo___log_call(MockClass<> const *call_ledger,
                            std::string mock_name,
                            std::string method_name,
                            std::size_t method_call_n) const {
      testudo___calls_p.push_back(
        {call_ledger, mock_name, method_name, method_call_n});
      for (auto const &parent: testudo___parents)
        parent.first->testudo___log_call(this, parent.second,
                                         method_name, method_call_n);
    }
    mutable std::vector<MockClassEntry> testudo___calls_p;
    std::list<std::pair<MockClass<> *, std::string>> testudo___parents;
  };

  using CallLedger=MockClass<>;

  template <class Base>
  class MockClass
    : public Base, public MockClass<> {
  public:
    using Base::Base;
  };

  inline void call_ledger_report_to_implementation(
      MockClass<> &child, MockClass<> *parent, std::string as)
    { child.report_to(parent, as); }
  template <typename CLP, typename=decltype(&*std::declval<CLP>())>
  void call_ledger_report_to_implementation(
      CLP child, MockClass<> *parent, std::string as)
    { call_ledger_report_to_implementation(*child, parent, as); }

  template <typename C>
  std::string print_calls(C const &c) {
    if (c.empty())
      return {};
    MockClassEntry const *strike_first=&c.front(), *strike_last=strike_first;
    bool first=true;
    std::string result=to_text(*strike_last);
    for (auto const &e: c) {
      if (not first) {
        if (not ((e.call_ledger==strike_last->call_ledger)
                 and (e.method_name==strike_last->method_name))) {
          if (strike_first not_eq strike_last)
            result+=" -- "+to_text(*strike_last);
          result+="\n"+to_text(e);
          strike_first=&e;
        }
        strike_last=&e;
      }
      first=false;
    }
    if (strike_first not_eq strike_last)
      result+=" -- "+to_text(*strike_last);
    result+="\n";
    return result;
  }

  // you can call "iterate()" with a reference to a call ledger, or a pointer,
  // or a smart pointer
  inline auto iterate(MockClass<> const &cl)
    { return MockClass<>::iterate_t{&cl}; }
  template <typename CLP, typename=decltype(&*std::declval<CLP>())>
  auto iterate(CLP clp) { return iterate(*clp); }


  // implementation for "testudo___IS_EMPTY()" adapted from
  // https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments
// #define testudo___HAS_COMMA(...)
#define testudo___TRIGGER_PARENTHESIS_(...) ,

#define testudo___ISEMPTY(...)                                          \
  testudo___ISEMPTY_IMPL(                                               \
    /* test if there is just one argument, eventually an empty one */   \
    testudo___HAS_COMMA(__VA_ARGS__),                                   \
    /* test if TRIGGER_PARENTHESIS_ together with the argument adds a   \
       comma */                                                         \
    testudo___HAS_COMMA(testudo___TRIGGER_PARENTHESIS_ __VA_ARGS__),    \
    /* test if the argument together with a parenthesis adds a comma */ \
    testudo___HAS_COMMA(__VA_ARGS__ (/*empty*/)),                       \
    /* test if placing it between TRIGGER_PARENTHESIS_ and the          \
       parenthesis adds a comma */                                      \
    testudo___HAS_COMMA(                                                \
      testudo___TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/))           \
  )

#define testudo___PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define testudo___ISEMPTY_IMPL(_0, _1, _2, _3)                          \
  testudo___HAS_COMMA(testudo___PASTE5(                                 \
    testudo___IS_EMPTY_CASE_, _0, _1, _2, _3))
#define testudo___IS_EMPTY_CASE_0001 ,


  // WITH_IS_EMPTY(abcd_, ) -> abcd_1
  // WITH_IS_EMPTY(abcd_, something) -> abcd_0
#define testudo___WITH_IS_EMPTY(base, ...)                              \
  testudo___CAT(base, testudo___ISEMPTY(__VA_ARGS__))


// for-each with numeric argument in reverse order; separate all invocations
// with commas; the action is the first argument:
// #define testudo___FOR_EACH_N_REV(...)

#define testudo___FOR_EACH_N_REV_BRACKETS_IS_EMPTY_1(action, ...)
#define testudo___FOR_EACH_N_REV_BRACKETS_IS_EMPTY_0(action, ...)    \
  testudo___FOR_EACH_N_REV(action, __VA_ARGS__)
// for-each with numeric argument in reverse order, with arguments grouped in
// brackets; works with empty list of arguments
#define testudo___FOR_EACH_N_REV_BRACKETS(action, b_args)               \
  testudo___CAT(testudo___FOR_EACH_N_REV_BRACKETS_IS_EMPTY_,            \
                testudo___ISEMPTY(testudo___REMOVE_BRACKETS b_args))    \
    (action, testudo___REMOVE_BRACKETS b_args)


#define testudo___PRODUCE_FULL_ARG(_, ...)                              \
  testudo___REMOVE_BRACKETS __VA_ARGS__
#define testudo___ARGS(b_args)                                          \
  testudo___FOR_EACH_N_REV_BRACKETS(testudo___PRODUCE_FULL_ARG, b_args)
  // SIGNATURE((int), ((char c), (float f))) -> int (char c, float f)
#define testudo___SIGNATURE(b_ret, b_args)                              \
  testudo___REMOVE_BRACKETS b_ret (testudo___ARGS(b_args))

  // ARGS_TYPE_NAME((int), ((char c), (float f))) -> char arg_2, float arg_1
#define testudo___TYPE_FROM_TYPE_NAME(...)                              \
  testudo___implementation::arg_t<void (__VA_ARGS__), 1>
#define testudo___PRODUCE_ARG_TYPE_NAME_N(n, ...)                       \
  testudo___TYPE_FROM_TYPE_NAME __VA_ARGS__ arg_##n
#define testudo___ARGS_TYPE_NAME(b_args)                                \
  testudo___FOR_EACH_N_REV_BRACKETS(testudo___PRODUCE_ARG_TYPE_NAME_N,  \
                                    b_args)

  // ARGS_NAME((int), ((char c), (float f))) -> arg_2, arg_1
#define testudo___PRODUCE_ARG_NAME_N(n, ...) arg_##n
#define testudo___ARGS_NAME(b_args)                                     \
  testudo___FOR_EACH_N_REV_BRACKETS(testudo___PRODUCE_ARG_NAME_N,       \
                                    b_args)

#define testudo___ARG3(_1, _2, _3, ...) _3

#define testudo__MOCK_METHOD(b_ret, name, ...)                          \
  testudo___ARG3(__VA_ARGS__,                                           \
                 testudo___MOCK_METHOD_IMPL,                            \
                 testudo___MOCK_METHOD_IMPL_NO_SPEC, )(                 \
    b_ret, name, __VA_ARGS__)
#define testudo___MOCK_METHOD_IMPL_NO_SPEC(b_ret, name, b_args)         \
  testudo___MOCK_METHOD_IMPL(b_ret, name, b_args, )
#define testudo___MOCK_METHOD_IMPL(b_ret, name, b_args, b_spec)         \
  testudo___MOCK_METHOD_IMPL_SPEC(b_ret, name, b_args, (b_spec))
#define testudo___MOCK_METHOD_IMPL_SPEC(b_ret, name, b_args, b_spec)    \
  testudo___REMOVE_BRACKETS b_ret                                       \
  testudo___SINGLE_OR_1ST(name)(testudo___ARGS_TYPE_NAME(b_args))       \
  testudo___REMOVE_BRACKETS b_spec {                                    \
    this->log_call(                                                     \
      testudo___STRING(testudo___SINGLE_OR_2ND(name)),                  \
      testudo___CAT(testudo___SINGLE_OR_2ND(name), _log).size());       \
    return get_return_value_and_log(                                    \
             testudo___CAT(testudo___SINGLE_OR_2ND(name), _schedule),   \
             testudo___CAT(testudo___SINGLE_OR_2ND(name), _log),        \
             {testudo___ARGS_NAME(b_args)});                            \
  }                                                                     \
  mutable testudo___implementation::RetArgsLog<                         \
      testudo___SIGNATURE(b_ret, b_args)>                               \
    testudo___CAT(testudo___SINGLE_OR_2ND(name), _log);                 \
  mutable testudo___implementation::Schedule<                           \
      testudo___SIGNATURE(b_ret, b_args)>                               \
    testudo___CAT(testudo___SINGLE_OR_2ND(name), _schedule)


  template <typename S, typename F>
  auto evaluate_to_tuple(F const &f, args_t<S> const &tu) {
    if constexpr (std::is_void_v<decltype(std::apply(f, tu))>) {
      std::apply(f, tu);
      return make_ret();
    }
    else {
      auto r=std::apply(f, tu);
      return make_ret(r);
    }
  }

#define testudo__WRAP_METHOD(b_ret, name, ...)                          \
  testudo___ARG3(__VA_ARGS__,                                           \
                 testudo___WRAP_METHOD_IMPL,                            \
                 testudo___WRAP_METHOD_IMPL_NO_SPEC, )(                 \
    b_ret, name, __VA_ARGS__)
#define testudo___WRAP_METHOD_IMPL_NO_SPEC(b_ret, name, b_args)         \
  testudo___WRAP_METHOD_IMPL(b_ret, name, b_args, )
#define testudo___WRAP_METHOD_IMPL(b_ret, name, b_args, b_spec)         \
  testudo___WRAP_METHOD_IMPL_SPEC(b_ret, name, b_args, (b_spec))
#define testudo___WRAP_METHOD_IMPL_SPEC(b_ret, name, b_args, b_spec)    \
  mutable testudo___implementation::RetArgsLog<                         \
      testudo___SIGNATURE(b_ret, b_args)>                               \
    testudo___CAT(testudo___SINGLE_OR_2ND(name), _log);                 \
  testudo___REMOVE_BRACKETS b_ret                                       \
  testudo___SINGLE_OR_1ST(name)(testudo___ARGS_TYPE_NAME(b_args))       \
  testudo___REMOVE_BRACKETS b_spec {                                    \
    log_call(                                                           \
      testudo___STRING(testudo___SINGLE_OR_2ND(name)),                  \
      testudo___CAT(testudo___SINGLE_OR_2ND(name), _log).size());       \
    auto &r=                                                            \
      testudo___CAT(testudo___SINGLE_OR_2ND(name), _log).log_values(    \
        {testudo___ARGS_NAME(b_args)});                                 \
    r=testudo___implementation::evaluate_to_tuple<                      \
          testudo___SIGNATURE(b_ret, b_args)>(                          \
        [this](testudo___ARGS_TYPE_NAME(b_args)) {                      \
          return testudo___CAT(                                         \
            testudo___SINGLE_OR_2ND(name),                              \
            _testudo___implementation)(testudo___ARGS_NAME(b_args));    \
        },                                                              \
        {testudo___ARGS_NAME(b_args)});                                 \
    return testudo___implementation::get_ret(r);                        \
  }                                                                     \
  testudo___REMOVE_BRACKETS b_ret                                       \
  testudo___CAT(testudo___SINGLE_OR_2ND(name),                          \
                _testudo___implementation)                              \
    (testudo___ARGS(b_args))


#define testudo__LOGGED_ARGS(name) name##_log.arg_values()
#define testudo__LOGGED_RET(name) name##_log.ret_values()
#define testudo__LOGGED_RET_ARGS(name) name##_log.ret_args_values()
#define testudo__LOG_SIZE(name) name##_log.size()
#define testudo__SCHEDULE_RET(name, ...) \
  name##_schedule.push_back(__VA_ARGS__)
#define testudo__SET_RET_DEFAULT(name, ...) \
  name##_schedule.set_default(__VA_ARGS__)
#define testudo__GET_CALL(mock, name)                                   \
  get_call_implementation(&(mock), #name, (mock).name##_log)
#define testudo__POP_CALL(mock, name)                                   \
  pop_call_implementation(&(mock), #name, (mock).name##_log)
#define testudo__CALL_LEDGER_REPORT_TO(child, parent)                   \
  testudo___implementation::call_ledger_report_to_implementation(       \
    child, parent, std::string(#child))


  namespace implementation {

    template <typename C, typename A,
              typename=decltype(std::declval<typename C::value_type>()
                                ==std::declval<A>())>
    bool is_always(C const &c, A const &a) {
      for (auto const &e: c)
        if (e not_eq a)
          return false;
      return true;
    }

    template <typename C, typename... A>
    bool is_always(C const &c, A const &...a)
      { return is_always(c, std::tuple{a...}); }

    template <typename C, typename A,
              typename=decltype(std::declval<typename C::value_type>()
                                ==std::declval<A>())>
    bool is_never(C const &c, A const &a) {
      for (auto const &e: c)
        if (e==a)
          return false;
      return true;
    }

    template <typename C, typename... A>
    bool is_never(C const &c, A const &...a)
      { return is_never(c, std::tuple{a...}); }

    template <typename C>
    bool is_constant(C const &c) {
      if (c.size()<2)
        return true;
      else
        return is_always(c, c.front());
    }

    template <typename C>
    bool all_different(C const &c) {
      std::unordered_set<typename C::value_type> checked;
      for (auto const &e: c) {
        if (checked.count(e))
          return false;
        checked.insert(e);
      }
      return true;
    }

  }

  template <typename... T>
  inline auto is_always(T const &...t) {
    return testudo__PREDICATE_C_A((t...), implementation::is_always(a, t...));
  }

  template <typename... T>
  inline auto is_never(T const &...t) {
    return testudo__PREDICATE_C_A((t...), implementation::is_never(a, t...));
  }

  inline auto is_constant=
    testudo__PREDICATE_A(implementation::is_constant(a));
  inline auto all_different=
    testudo__PREDICATE_A(implementation::all_different(a));

}

testudo___BRING(MockClass,
                CallLedger,
                void_v,
                throw_exception,
                is_always,
                is_never,
                is_constant,
                all_different)

#endif
