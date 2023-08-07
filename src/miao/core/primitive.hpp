#pragma once
#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

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
  virtual const bsdf *get_material() const = 0;
};

class GeoPrimitive : public primitive {
public:
  GeoPrimitive(const std::shared_ptr<shape> &s,
               const std::shared_ptr<bsdf> &mat,
               const std::shared_ptr<AreaLight> &light)
      : s(s), mat(mat), areaLight(light) {}
  virtual const AreaLight *get_area_light() const override {
    return areaLight.get();
  }
  virtual const bsdf *get_material() const override { return mat.get(); }
  virtual BBox worldbound() const override;
  virtual std::optional<SurfaceInteraction> intersect(const ray &r,
                                                      double &t) const override;

private:
  std::shared_ptr<shape> s;
  std::shared_ptr<bsdf> mat;
  std::shared_ptr<AreaLight> areaLight;
  // medium interface here (but ignoring for now)
};

class aggregate : public primitive {
public:
  virtual const AreaLight *get_area_light() const override;
  virtual const bsdf *get_material() const override;
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
