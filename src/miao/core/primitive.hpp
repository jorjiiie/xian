#pragma once
#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include <memory>
#include <optional>
#include <vector>

#include "miao/core/BBox.hpp"
#include "miao/core/interaction.hpp"

namespace miao {

class AreaLight;
class material;
class shape;

class primitive {
public:
  // virtual ~primitive();
  virtual BBox worldbound() const = 0;
  virtual std::optional<SurfaceInteraction> intersect(const ray &r,
                                                      double &) const = 0;

  virtual const AreaLight *get_area_light() const = 0;
  virtual const material *get_material() const = 0;
};

class GeoPrimitive : public primitive {
public:
  GeoPrimitive(const std::shared_ptr<shape> &s_,
               const std::shared_ptr<material> &mat_,
               const std::shared_ptr<AreaLight> &light_)
      : s(s_), mat(mat_), areaLight(light_) {}
  virtual const AreaLight *get_area_light() const override {
    return areaLight.get();
  }
  virtual const material *get_material() const override { return mat.get(); }
  virtual BBox worldbound() const override;
  virtual std::optional<SurfaceInteraction> intersect(const ray &r,
                                                      double &t) const override;

private:
  std::shared_ptr<shape> s;
  std::shared_ptr<material> mat;
  std::shared_ptr<AreaLight> areaLight;
  // medium interface here (but ignoring for now)
};

class aggregate : public primitive {
public:
  virtual const AreaLight *get_area_light() const override;
  virtual const material *get_material() const override;
};

// loop aggregate!
class dumb_aggregate : public aggregate {
public:
  dumb_aggregate(std::vector<GeoPrimitive> &prims) : p(prims) {}
  virtual BBox worldbound() const override;
  virtual std::optional<SurfaceInteraction> intersect(const ray &r,
                                                      double &) const override;

private:
  BBox wb;
  std::vector<GeoPrimitive> p;
};

} // namespace miao

#endif
