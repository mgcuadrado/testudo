#ifndef MGCUADRADO_TESTUDO_lc_STYLE_HEADER_
#define MGCUADRADO_TESTUDO_lc_STYLE_HEADER_

#include "testudo.h"

#define top_test_node(...) \
  testudo__TOP_TEST_NODE(__VA_ARGS__)
#define define_top_test_node(...) \
  testudo__DEFINE_TOP_TEST_NODE(__VA_ARGS__)
#define define_test_node(...) \
  testudo__DEFINE_TEST_NODE(__VA_ARGS__)

#define test_parameters \
  testudo__TEST_PARAMETERS
#define test_arguments \
  testudo__TEST_ARGUMENTS

#define define_top_test(...) \
  testudo__DEFINE_TOP_TEST(__VA_ARGS__)
#define define_test(...) \
  testudo__DEFINE_TEST(__VA_ARGS__)
#define with_fixture(...) \
  testudo__WITH_FIXTURE(__VA_ARGS__)

#define with_data_define_top_test(...) \
  testudo__WITH_DATA_DEFINE_TOP_TEST(__VA_ARGS__)
#define with_data_define_test(...) \
  testudo__WITH_DATA_DEFINE_TEST(__VA_ARGS__)

#define print_text(...) \
  testudo__PRINT_TEXT(__VA_ARGS__)
#define print_multiline_text(...) \
  testudo__PRINT_MULTILINE_TEXT(__VA_ARGS__)
#define print_break(...) \
  testudo__PRINT_BREAK(__VA_ARGS__)

#define show_scope(...) \
  testudo__SHOW_SCOPE(__VA_ARGS__)
#define declare(...) \
  testudo__DECLARE_S(#__VA_ARGS__, __VA_ARGS__)
#define perform(...) \
  testudo__PERFORM_S(#__VA_ARGS__, __VA_ARGS__)
#define fake_declare(...) \
  testudo__FAKE_DECLARE_S(#__VA_ARGS__, __VA_ARGS__)
#define fake_perform(...) \
  testudo__FAKE_PERFORM_S(#__VA_ARGS__, __VA_ARGS__)
#define check_try_catch(flag, ...) \
  testudo__CHECK_TRY_CATCH_S(flag, #__VA_ARGS__, __VA_ARGS__)
#define show_value(...) \
  testudo__SHOW_VALUE_S(#__VA_ARGS__, __VA_ARGS__)
#define show_multiline_value(...) \
  testudo__SHOW_MULTILINE_VALUE_S(#__VA_ARGS__, __VA_ARGS__)

#define define_approx_epsilon(...) \
  testudo__DEFINE_APPROX_EPSILON(__VA_ARGS__)
#define set_approx_epsilon(...) \
  testudo__SET_APPROX_EPSILON(__VA_ARGS__)
#define show_approx_epsilon(...) \
  testudo__SHOW_APPROX_EPSILON(__VA_ARGS__)

#define check(...) \
  testudo__CHECK_S(#__VA_ARGS__, __VA_ARGS__)
#define _equal(...) \
  testudo__EQUAL_S(#__VA_ARGS__, __VA_ARGS__)
#define _approx(...) \
  testudo__APPROX_S(#__VA_ARGS__, __VA_ARGS__)
#define _tol(...) \
  testudo__TOL_S(#__VA_ARGS__, __VA_ARGS__)
#define _true \
  testudo__TRUE

#endif
