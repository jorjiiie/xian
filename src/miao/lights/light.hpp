#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "miao/core/ray.hpp"
#include "miao/core/spectrum.hpp"

namespace miao {
class light {
public:
  virtual spectrum Le(const ray &r) const = 0;
};

class AreaLight : public light {
public:
  AreaLight(const spectrum &s) : emit(s) {}
  virtual spectrum Le(const ray &r) const override { return emit; }
  // le is gonna be the same
private:
  spectrum emit;
};
} // namespace miao

#endif // LIGHT_HPP
