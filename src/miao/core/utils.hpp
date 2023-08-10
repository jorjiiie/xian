#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <cmath>
#include <limits>

namespace miao {
#define PI 3.1415926534
#define INV_PI 0.318309886184
#define EPS 1e-6

const double D_INFINITY = std::numeric_limits<double>::max();

struct cnter_ {
  static int64_t prim_tests;
  static int64_t bbox_tests;
};

// max of two Ts
template <typename T> inline T max(const T &a, const T &b) {
  return (a < b ? b : a);
}

// min of two Ts
template <typename T> inline T min(const T &a, const T &b) {
  return (b < a ? b : a);
}

// sets first parameter to max of two Ts and returns reference to first
// parameter
template <typename T> inline T &chmax(T &a, const T &b) {
  return (a = max(a, b));
}

// sets first parameter to min of two Ts and returns reference to first
// parameter
template <typename T> inline T &chmin(T &a, const T &b) {
  return (a = min(a, b));
}

// interpolates between [a,b] for t in [0,1]
inline double lerp(double t, double a, double b) {
  return (1.0 - t) * a + t * b;
}

template <typename T> inline T clamp(T a, T b, T c) {
  return max(min(b, c), a);
}

inline bool quadratic(double a, double b, double c, double &s0, double &s1) {
  double disc = b * b - 4 * a * c;
  if (disc < 0)
    return false;
  double rt = std::sqrt(disc);

  // pbrt numerically stable form of quadratic eqn (can try default later as
  // experiment)
  double q;
  if (b < 0)
    q = -.5 * (b - rt);
  else
    q = -.5 * (b + rt);

  s0 = q / a;
  s1 = c / q;
  if (s0 > s1)
    std::swap(s0, s1);
  return true;
}
} // namespace miao

#endif
