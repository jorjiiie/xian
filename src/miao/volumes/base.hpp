#pragma once
#ifndef VOLUME_BASE_HPP
#define VOLUME_BASE_HPP

#include "miao/core/rng.hpp"
#include "miao/core/vec3.hpp"

namespace miao {

class PhaseFunction {
public:
  virtual double p(const vec3 &, const vec3 &) const = 0;
  virtual double sample_p(const vec3 &, vec3 &, RNG &rng) const = 0;
};

class HenyeyGreenstein : public PhaseFunction {
public:
  HenyeyGreenstein(double g_) : g(g_) {}
  double p(const vec3 &wi, const vec3 &wo) const {
    double denom = 1 + g * g + 2 * g * vec3::dot(wi, wo);
    return INV_PI * .25 * (1 - g * g) / (denom * std::sqrt(denom));
  }

private:
  double g; // param
};

class isotropic : public PhaseFunction {
public:
  virtual double p(const vec3 &, const vec3 &) const {
    return INV_PI * 0.25;
  } // namespace miao
  virtual double sample_p(const vec3 &, vec3 &wi, RNG &rng) const {
    double t = rng.rfloat() * 2 * PI;
    double p = rng.rfloat() * PI;

    wi.x = std::sin(p) * std::cos(t);
    wi.y = std::sin(p) * std::sin(t);
    wi.z = std::cos(p);
    return INV_PI * 0.25; // this is the fraction that gets sent!
  }
};

} // namespace miao

#endif
