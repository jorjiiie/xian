#pragma once
#ifndef VOLUME_BASE_HPP
#define VOLUME_BASE_HPP

#include "miao/core/rng.hpp"
#include "miao/core/vec3.hpp"

namespace miao {

class PhaseFunction {
  virtual double p(const vec3 &, const vec3 &) const = 0;
  virtual double sample_p(const vec3 &, vec3 &, RNG &rng) = 0;
};

class HenyeyGreenstein : public PhaseFunction {
public:
  HenyeyGreenstein(double g_) : g(g_) {}

  // ok!
  double p(const vec3 &wi, const vec3 &wo) const {
    double denom = 1 + g * g + 2 * g * vec3::dot(wi, wo);
    return INV_PI * .25 * (1 - g * g) / (denom * std::sqrt(denom));
  }

private:
  double g; // param
};

} // namespace miao

#endif
