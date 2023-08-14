#pragma once
#ifndef MEDIUM_HPP
#define MEDIUM_HPP

#include "miao/core/ray.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/spectrum.hpp"

#include <cmath>

namespace miao {

class MediumInteraction;

class medium {
public:
  // virtual ~medium();
  virtual spectrum tr(const ray &r, double len, RNG &rng) const = 0;
  // gives next point where we should do something? - max length IS stored in r
  virtual spectrum sample(const ray &r, RNG &rng,
                          MediumInteraction &mi) const = 0;
};

class homogeneous : public medium {
public:
  homogeneous(const spectrum &sigma_a, const spectrum &sigma_s, double g_)
      : sa(sigma_a), ss(sigma_s), st(sigma_a + sigma_s), g(g_) {}
  virtual spectrum tr(const ray &r, double len, RNG &) const override {
    return (-st * len).exp();
  }

  virtual spectrum sample(const ray &r, RNG &rng,
                          MediumInteraction &mi) const override;

  // private:
  //  absorb, scatter, both
  const spectrum sa, ss, st;
  const double g;
};

class MediumInterface {
public:
  // maybe should have one for air? or nah who cares
  MediumInterface() : in(nullptr), out(nullptr) {}
  MediumInterface(const medium *med) : in(med), out(med) {}
  MediumInterface(const medium *a, const medium *b) : in(a), out(b) {}
  bool transition() const { return in != out; }

  const medium *in, *out;
};
} // namespace miao

#endif
