#pragma once
#ifndef RAY_HPP
#define RAY_HPP

#include "vec3.hpp"

namespace miao {
class medium;

struct ray {
  ray() : mint(0.0), maxt(D_INFINITY), time(0.0) {}
  ray(const point3 &origin, const vec3 &dir, double start,
      double end = D_INFINITY, double t = 0.0, const medium *med = nullptr)
      : o(origin), d(dir), mint(start), maxt(end), time(t), m(med) {}
  point3 o;
  vec3 d;
  double mint, maxt;
  double time;
  const medium *m;
};

} // namespace miao
#endif
