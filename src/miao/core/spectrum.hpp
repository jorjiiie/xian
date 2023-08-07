#pragma once
#ifndef SPECTRUM_HPP
#define SPECTRUM_HPP

#include <cmath>

#include "utils.hpp"

namespace miao {
template <int n> class BaseSpectrum {
public:
  BaseSpectrum(double v = 0.0) {
    for (int i = 0; i < n; i++)
      c[i] = v;
  }
  BaseSpectrum(const BaseSpectrum &s) {
    for (int i = 0; i < n; i++)
      c[i] = s.c[i];
  }
  BaseSpectrum &operator=(const BaseSpectrum &s) {
    for (int i = 0; i < n; i++)
      c[i] = s.c[i];
    return *this;
  }

  BaseSpectrum &operator+=(const BaseSpectrum &s) {
    for (int i = 0; i < n; i++)
      c[i] += s.c[i];
    return *this;
  }
  BaseSpectrum operator+(const BaseSpectrum &s) const {
    BaseSpectrum r = *this;
    r += s;
    return r;
  }
  BaseSpectrum &operator-=(const BaseSpectrum &s) {
    for (int i = 0; i < n; i++)
      c[i] -= s.c[i];
    return *this;
  }
  BaseSpectrum operator-(const BaseSpectrum &s) const {
    BaseSpectrum r = *this;
    r -= s;
    return r;
  }
  BaseSpectrum &operator*=(const BaseSpectrum &s) {
    for (int i = 0; i < n; i++)
      c[i] *= s.c[i];
    return *this;
  }
  BaseSpectrum operator*(const BaseSpectrum &s) const {
    BaseSpectrum r = *this;
    r *= s;
    return r;
  }
  BaseSpectrum &operator/=(const BaseSpectrum &s) {
    for (int i = 0; i < n; i++)
      c[i] /= s.c[i];
    return *this;
  }
  BaseSpectrum operator/(const BaseSpectrum &s) const {
    BaseSpectrum r = *this;
    r /= s;
    return r;
  }
  BaseSpectrum operator-() const {
    BaseSpectrum r = *this;
    for (int i = 0; i < n; i++)
      r.c[i] = -r.c[i];
    return r;
  }
  bool black() const {
    for (int i = 0; i < n; i++)
      if (c[i] != 0.0)
        return false;
    return true;
  }
  double &operator[](int i) { return c[i]; }
  double operator[](int i) const { return c[i]; }
  bool operator==(const BaseSpectrum &s) const {
    for (int i = 0; i < n; i++)
      if (c[i] != s.c[i]) // might wanna use epsilon here
        return false;
    return true;
  }
  bool operator!=(const BaseSpectrum &s) const { return !(*this == s); }

  BaseSpectrum sqrt() const {
    BaseSpectrum r = *this;
    for (int i = 0; i < n; i++)
      r.c[i] = std::sqrt(r.c[i]);
    return r;
  }

  BaseSpectrum clamp(double low = 0.0, double high = D_INFINITY) const {
    BaseSpectrum r = *this;
    for (int i = 0; i < n; i++)
      r.c[i] = clamp(r.c[i], low, high);
    return r;
  }
  bool isBlack() const {
    for (int i = 0; i < n; i++) {
      if (c[i] != 0.0)
        return false;
    }
    return true;
  }
  double magnitude() const {
    double r = 0;
    for (int i = 0; i < n; i++)
      r += c[i] * c[i];
    return std::sqrt(r);
  }
  bool hasNaNs() const {
    for (int i = 0; i < n; i++)
      if (std::isnan(c[i]))
        return true;
    return false;
  }
  virtual void toRGB(double &r, double &g, double &b) const { r = g = b = 0; };

  static const int nSamples = n;

protected:
  double c[n];
};

class RGBSpectrum : public BaseSpectrum<3> {
public:
  RGBSpectrum() {}
  RGBSpectrum(double d) : BaseSpectrum<3>(d) {}
  RGBSpectrum(double r, double g, double b) {
    c[0] = r;
    c[1] = g;
    c[2] = b;
  }
  RGBSpectrum(const BaseSpectrum<3> &s) : BaseSpectrum<3>(s) {}
  virtual void toRGB(double &r, double &g, double &b) const override {
    r = c[0];
    g = c[1];
    b = c[2];
  }
};
// typedef BaseSpectrum<10> spectrum;
typedef RGBSpectrum spectrum;

} // namespace miao
#endif // spectrum_hpp
