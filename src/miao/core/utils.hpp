#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <limits>

#define PI 3.1415926534
#define INV_PI 0.318309886184
#define EPS 1e-6

const double D_INFINITY = std::numeric_limits<double>::max();

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

#endif
