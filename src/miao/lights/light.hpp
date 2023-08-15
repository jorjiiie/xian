#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "miao/core/interaction.hpp"
#include "miao/core/ray.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/shape.hpp"
#include "miao/core/spectrum.hpp"

#include <memory>

namespace miao {
class visibility;
class light {
public:
  virtual spectrum Le(const ray &r) const = 0;
  // assumes that this spot is visible!
  virtual spectrum Li(const interaction &si, RNG &rng, vec3 &wi, double &pdf,
                      visibility *v) const = 0;

  virtual double pdf_li(const interaction &si, const vec3 &wi) const = 0;
  virtual spectrum sample(ray &r, RNG &rng, double &pdf) const = 0;
  virtual spectrum power() const = 0;
  // for leaving the light
  const medium *m = nullptr;
};

class AreaLight : public light {
public:
  AreaLight(const spectrum &s, std::shared_ptr<shape> b_)
      : emit(s), base(b_), area(b_->area()) {}
  virtual spectrum Le(const ray &) const override { return emit; }
  virtual spectrum Li(const interaction &si, RNG &rng, vec3 &wi, double &pdf,
                      visibility *v) const override;

  virtual double pdf_li(const interaction &si, const vec3 &wi) const override;
  virtual spectrum sample(ray &r, RNG &rng, double &pdf) const override;
  virtual spectrum power() const override;
  // le is gonna be the same
private:
  spectrum emit;
  std::shared_ptr<shape> base;
  double area;
};

} // namespace miao

#endif // LIGHT_HPP
