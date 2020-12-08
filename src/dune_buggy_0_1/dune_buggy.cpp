#include "dune_buggy.h"
#include "../testudo_base.h"
#include <iostream>

namespace dune_buggy {

  using namespace std;

  location::location(double x, double y, double a)
    : x(x), y(y), a(a) { }

  ostream &operator<<(ostream &os, location p)
    { return os << "{" << p.x << ", " << p.y << ", " << p.a << "}"; }

  // dummy implementation
  location operator-(location) { return origin; }

  // dummy implementation
  location operator+(location p, location q) {
    return {p.x+q.x*cos(p.a)-q.y*sin(p.a),
            p.y+q.x*sin(p.a)+q.y*cos(p.a),
            p.a+q.a};
  }

  double absdiff_testudo(location p, location q) {
    return (testudo::absdiff(p.x, q.x)
            +testudo::absdiff(p.y, q.y)
            +testudo::absdiff(p.a, q.a));
  }

}
