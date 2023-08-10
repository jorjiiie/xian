#pragma once
#include "miao/core/utils.hpp"
#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "miao/core/BBox.hpp"
#include "miao/core/interaction.hpp"
#include "miao/core/ray.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/transform.hpp"
#include "miao/core/vec3.hpp"

namespace miao {
// for now, shapes will be the homebaked solution (but in theory supports
// transforms)
//
vec3 uniform_sphere(RNG &rng);
double uniform_sphere_pdf();

class shape {
public:
  shape(const Transformation *otw, const Transformation *wto, bool reverse)
      : otw(otw), wto(wto), r(reverse) {}

  virtual BBox getBBox() const = 0;

  virtual BBox worldBBox() const { return (*otw)(getBBox()); }

  virtual bool intersect(const ray &r, double &t,
                         SurfaceInteraction &isect) const = 0;

  virtual bool intersecthuh(const ray &r_) const {
    double t = r_.maxt;
    SurfaceInteraction i{};
    return intersect(r_, t, i);
  }

  virtual double area() const = 0;

  // i think these are just rng
  virtual interaction sample(RNG &r) const = 0;
  virtual double pdf(const interaction &) const { return 1.0 / area(); }

  virtual interaction sample(const interaction &, RNG &rng) const {
    return sample(rng);
  }
  virtual double pdf(const interaction &r, const vec3 &wi) const = 0;

  const Transformation *otw, *wto;
  bool r;
};

class sphere : public shape {
public:
  sphere(const Transformation *otw, const Transformation *wto, bool rev,
         double r, double zmin, double zmax, double tmin, double tmax,
         double pmax)
      : shape(otw, wto, rev), radius(r), zmin(clamp(zmin, -r, r)),
        zmax(clamp(zmax, -r, r)), tmin(std::acos(clamp(zmin / r, -1.0, 1.0))),
        tmax(clamp(zmax / r, -1.0, 1.0)), pmax(clamp(pmax, 0.0, 360.0)),
        origin((*otw)(point3{}, true)) {}
  virtual BBox getBBox() const override;
  virtual bool intersect(const ray &r, double &t,
                         SurfaceInteraction &isect) const override;
  virtual double area() const override;
  virtual interaction sample(RNG &r) const override;
  virtual interaction sample(const interaction &i, RNG &r) const override;
  virtual double pdf(const interaction &r, const vec3 &wi) const override;

  // private:
  double radius;
  double zmin, zmax;
  double tmin, tmax, pmax;
  vec3 origin;
};
} // namespace miao
#endif
