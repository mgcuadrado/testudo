![logo](/doc/ascii_logo.png)

# Testudo, an automatic test system for C++ code

Testudo is a framework in which you can write automatic tests for your C++
code.  Its key features are:

- tests are written in pure C++, in a clear, concise, idiomatic way;

- the check system is extensible, and comprises by default bool checks,
  equality checks, approximated checks (for floating-point types), and
  expected-exception checks;

- you can make your new classes Testudo-aware easily, so that Testudo can use
  them for checks or print their values;

- Testudo detects unexpected exceptions, and counts them as errors;

- natural support for fixtures;

- natural support for testing with mocks;

- you can produce full, detailed test reports that can be understood on their
  own, without checking the source code;

- you can organise your tests as a tree;

- you can produce test summaries with success, fail, and error counts and
  accumulated tallies across the test tree;

- you can produce progress tracking reports, that show exactly and succintly
  how your code changes affect the test results.

# Installation

In order to use Testudo, you'll need a C++17 compiler at the very least.  If
you also have `bash`, `sed`, `awk`, `m4`, and `make`, you'll be able to use the
default deployment.

Installation procedure with the default deployment:

- `make install` will install Testudo under the `~/local` prefix; if you want a
  different prefix, use `make install PREFIX=<your prefix>`;

- make sure your `PREFIX/bin` directory is in your `PATH`.

# A simple example

Here's a very simple example of a Testudo test file, containing a test node and
a test:
```c++
#include <testudo/testudo_lc.h> // default lower-case style
#include "flux_capacitor.h"
#include "delorean_test.h" // defines DeloreanFixture

define_top_test_node("bttf1.outatime", // parent full name
                     (flux_capacitor, // test name
                      "FC features")); // title

define_test(flux_capacitor, // parent
            "FC after construction", // title
            with_fixture(DeloreanFixture)) { // use a fixture
  declare(FluxCapacitor fc);
  perform(fc.connect_to(delorean)); // "delorean" is created by the fixture
  check(not fc.is_on())_true; // check bool is true
  check(fc.missing_jw())_approx(1.21); // check approximate value
}
```

Here's an equivalent Testudo test file, expressed with a different style (there
are two default styles; you can also add your own):
```c++
#include <testudo/testudo_uc.h> // default upper-case style
#include "flux_capacitor.h"
#include "delorean_test.h" // defines DeloreanFixture

DEFINE_TOP_TEST_NODE("bttf2.outatime", // parent full name
                     (flux_capacitor, // test name
                      "FC features")); // title

DEFINE_TEST(flux_capacitor, // parent
            "FC after construction", // title
            WITH_FIXTURE(DeloreanFixture)) // use a fixture
{
  DECLARE(FluxCapacitor fc);
  PERFORM(fc.connect_to(delorean)); // "delorean" is created by the fixture
  CHECK(not fc.is_on()) TRUE; // check bool is true
  CHECK(fc.missing_jw()) APPROX(1.21); // check approximate value
}
```

And here's a possible output with one fail (somebody watched ze dubbed version
and didn't get zeir jigawatts right):
```
 _______________________________________________
| {testudo.outatime.flux_capacitor} FC features |
`-----------------------------------------------'
 _________________________________________________________________
| flux_capacitor_test.ttd:9                                       |
| {testudo.outatime.flux_capacitor.initial} FC after construction |
`-----------------------------------------------------------------'
" with fixture DeloreanFixture "
12 : FluxCapacitor fc ;
13 # fc.connect_to(delorean) ;
14 % not fc.is_on()                                                     [ OK ]
15 % fc.missing_jw() // 1.21 +/- eps : 2.21 // 1.21 ------------------- [FAIL]
{testudo.outatime.flux_capacitor.initial} 1/2 fail -------------------- [FAIL]

{testudo.outatime.flux_capacitor} 1/2 fail ---------------------------- [FAIL]
```
(only with colours, but `README.md` doesn't support colours).

As you can see, all details of the source code are transferred to the full
report, but results are added in the form of `[ OK ]` and `[FAIL]` tags, and
accumulated counts, and each line is coded with various characters to mark what
kind of a line it is (a declaration, an action, a check…).  Source code
filename and line information are also added to the report (see filename and
line in the test cartouche, and line numbers at the beginning of each test
step).  When a value doesn't match its expected value, both the actual value
and the expected value are printed, as in the check for `fc.missing_jw()`: you
see the actual value `2.21` and the expected value `1.21`.  In that same line,
`+/- eps` means the comparison used the current value for the tolerance; you
can also change the tolerance, or specify a tolerance for a specific check.

If you request just a summary instead of the full report, you get this:
```
{-.flux_capacitor} 1/2 fail ------------------------------------------- [FAIL]
    {-.initial} 1/2 fail ---------------------------------------------- [FAIL]
```

If you later discover the bug in your code, and correct it, you'll get the
following report:
```
 _______________________________________________
| {testudo.outatime.flux_capacitor} FC features |
`-----------------------------------------------'
 _________________________________________________________________
| flux_capacitor_test.ttd:9                                       |
| {testudo.outatime.flux_capacitor.initial} FC after construction |
`-----------------------------------------------------------------'
" with fixture DeloreanFixture "
12 : FluxCapacitor fc ;
13 # fc.connect_to(delorean) ;
14 % not fc.is_on()                                                     [ OK ]
15 % fc.missing_jw() // 1.21 +/- eps                                    [ OK ]
{testudo.outatime.flux_capacitor.initial} 0/2 fail -------------------- [ OK ]

{testudo.outatime.flux_capacitor} 0/2 fail ---------------------------- [ OK ]
```
and the following report summary:
```
{-.flux_capacitor} 0/2 fail ------------------------------------------- [ OK ]
    {-.initial} 0/2 fail ---------------------------------------------- [ OK ]
```
but more importantly, you can produce the following very concise progress
tracking report, where you can pinpoint exactly what your changes did to the
tests:
```
wrong to good (1: 1/1 f -> 0/1 f)
  [flux_capacitor_1_test.ttd:15 -> 15] c-check_approx (1/1 f -> 0/1 f)
```
(meaning your changes have caused one check to go from wrong to good, and that
check is in line 15 of the test file).

Testudo has many more features and syntax elements than i can show you
comfortably in this `README.md`.  Please refer to the
![guide](/doc/testudo_lc_cl.pdf).
