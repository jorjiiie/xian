#pragma once
#include "utils.hpp"
#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "Bbox.hpp"
#include "RNG.hpp"
#include "interaction.hpp"
#include "ray.hpp"
#include "transform.hpp"
#include "vec3.hpp"

// for now, shapes will be the homebaked solution (but in theory supports
// transforms)

class shape {
public:
  shape(const Transformation *otw, const Transformation *wto, bool reverse)
      : otw(otw), wto(wto), r(reverse) {}

  virtual BBox getBBox() const = 0;

  virtual BBox worldBBox() const { return (*otw)(getBBox()); }

  virtual bool intersect(const ray &r, double &t,
                         SurfaceInteraction &isect) const = 0;

  virtual bool intersecthuh(const ray &r) const {
    double t = r.maxt;
    SurfaceInteraction i{};
    return intersect(r, t, i);
  }

  virtual double area() const = 0;

  // i think these are just rng
  virtual interaction sample(RNG &r) const = 0;
  virtual double pdf(const interaction &) const { return 1.0 / area(); }

  virtual interaction sample(const interaction &i, RNG &r) const {
    return sample(r);
  }
  virtual double pdf(const interaction &r, const vec3 &wi) const;

  const Transformation *otw, *wto;
  bool r;
};

class sphere : public shape {
public:
  sphere(const Transformation *otw, const Transformation *wto, bool rev,
         double r, double zmin, double zmax, double tmin, double tmax,
         double pmax)
      : shape(otw, wto, rev), r(r), zmin(clamp(zmin, -r, r)),
        zmax(clamp(zmax, -r, r)), tmin(std::acos(clamp(zmin / r, -1.0, 1.0))),
        tmax(clamp(zmax / r, -1.0, 1.0)), pmax(clamp(pmax, 0.0, 360.0)) {}
  virtual BBox getBBox() const override;
  virtual bool intersect(const ray &r, double &t,
                         SurfaceInteraction &isect) const override;
  virtual bool intersecthuh(const ray &r) const override;
  virtual double area() const override;
  virtual interaction sample(RNG &r) const override;
  virtual interaction sample(const interaction &i, RNG &r) const override;
  virtual double pdf(const interaction &r, const vec3 &wi) const override;

private:
  double r;
  double zmin, zmax;
  double tmin, tmax, pmax;
};

#endif
