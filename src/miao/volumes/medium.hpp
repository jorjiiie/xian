#pragma once
#ifndef MEDIUM_HPP
#define MEDIUM_HPP

#include "miao/core/interaction.hpp"
#include "miao/core/ray.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/spectrum.hpp"

namespace miao {

class medium {
public:
  virtual ~medium();
  virtual spectrum tr(const ray &r, double len, RNG &rng) const = 0;
  virtual spectrum sample(const ray &r, RNG &rng,
                          MediumInteraction &mi) const = 0;
};

class MediumInterface {
public:
  MediumInterface(const medium *med) : in(med), out(med) {}
  MediumInterface(const medium *a, const medium *b) : in(a), out(b) {}
  bool transition() const { return in != out; }

private:
  const medium *in, *out;
};
} // namespace miao

#endif
