#ifndef MGCUADRADO_TESTUDO_DUNE_BUGGY_HEADER_
#define MGCUADRADO_TESTUDO_DUNE_BUGGY_HEADER_

#include <iosfwd>
#include <cmath>

namespace dune_buggy {

  double constexpr pi=acos(-1.);

  // 2D location
  struct location {
    location(double x, double y, double a);
    double x, y, a; // coordinates 'x' and 'y', and angle 'a'
  };
  location const origin{0., 0., 0.};
  std::ostream &operator<<(std::ostream &, location);

  // "p+(-p)==(-p)+p==origin"
  location operator-(location p);
  // 'q' is expressed relative to 'p'
  location operator+(location p, location q);

  /// Testudo
  double absdiff_testudo(location, location);

}

#endif
