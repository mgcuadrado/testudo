define(`forloop', `pushdef(`$1', `$2')_forloop($@)popdef(`$1')')`'dnl
define(`_forloop',
       `$4`'ifelse($1, `$3', `', `define(`$1', incr($1))$0($@)')')`'dnl
define(`revforloop', `pushdef(`$1', `$2')_revforloop($@)popdef(`$1')')`'dnl
define(`_revforloop',
       `$4`'ifelse($1, `$3', `', `define(`$1', decr($1))$0($@)')')`'dnl
dnl
include(`testudo_macros_n_customize.m4')dnl
`// auto-generated file; generated from "testudo_macros_n.m4"'

`#define testudo___GET_MACRO_'incr(max_n)`(action, \
  'forloop(`i', 1, max_n, ``_'i`, \
  '')`name, ...) name'

`#define testudo___FOR_EACH_0(what)
#define testudo___FOR_EACH_1(what, x) \
  what(x)'
dnl
forloop(`i', 2, max_n, ``#define testudo___FOR_EACH_'i`(what, x, ...) \
  what(x)testudo___FOR_EACH_'decr(i)`(what, __VA_ARGS__)
'')
dnl
`// the action is the first argument:
#define testudo___FOR_EACH(...) \
  testudo___GET_MACRO_'incr(max_n)`( \
    __VA_ARGS__, \
    'revforloop(`i', max_n, 1, ``testudo___FOR_EACH_'i`, \
    '')`testudo___FOR_EACH_0)(__VA_ARGS__)'


`#define testudo___COMMA_REST_0()
#define testudo___COMMA_REST_1(X)
#define testudo___COMMA_REST_SEVERAL(X, ...) , __VA_ARGS__'

`#define testudo___COMMA_REST(...) \
  testudo___GET_MACRO_'incr(max_n)`( \
    __VA_ARGS__, \
    'forloop(`i', 1, max_n, ``testudo___COMMA_REST_SEVERAL, /* 'i` */ \
    '')`testudo___COMMA_REST_1, )(__VA_ARGS__)'

`#define testudo___ARG'max_n`( \
  'forloop(`i', 1, max_n, ``_'i`, \
  '')`...) _'max_n

`#define testudo___HAS_COMMA(...) \
  testudo___ARG'max_n`( \
    __VA_ARGS__, \
    'forloop(`i', 2, decr(max_n), ``1, /* 'i` */ \
    '')`0, /* 'max_n` */ \
    _)'


`#define testudo___FOR_EACH_N_REV_0(what)
#define testudo___FOR_EACH_N_REV_1(what, x) \
  what(1, x)'
dnl
forloop(`i', 2, max_n, ``#define testudo___FOR_EACH_N_REV_'i`(what, x, ...) \
  what('i`, x), testudo___FOR_EACH_N_REV_'decr(i)`(what, __VA_ARGS__)
'')
`#define testudo___FOR_EACH_N_REV(...) \
  testudo___GET_MACRO_'incr(max_n)`( \
    __VA_ARGS__, 'revforloop(`i', max_n, 0, ``\
    testudo___FOR_EACH_N_REV_'i`, '')`\
  )(__VA_ARGS__)'
