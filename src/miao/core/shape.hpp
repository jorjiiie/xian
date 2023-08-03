#pragma once
#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "Bbox.hpp"
#include "RNG.hpp"
#include "ray.hpp"
#include "transform.hpp"
#include "vec3.hpp"

// for now, shapes will be the homebaked solution (but in theory supports
// transforms)

class interaction {};
class SurfaceInteraction : public interaction {};

class shape {
public:
  shape(const Transformation *otw, const Transformation *wto, bool reverse)
      : otw(otw), wto(wto), r(reverse) {}

  virtual BBox getBBox() const = 0;

  virtual BBox worldBBox() const { return (*otw)(getBBox()); }

  virtual bool intersect(const ray &r, double &t,
                         SurfaceInteraction &isect) const = 0;

  virtual bool intersecthuh(const ray &r) {
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

#endif
