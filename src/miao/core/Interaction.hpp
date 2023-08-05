#pragma once
#include "utils.hpp"
#ifndef INTERACTION_HPP
#define INTERACTION_HPP

#include "ray.hpp"
#include "vec3.hpp"

namespace miao {
class MediumInterface;
class shape;
class primitive;
class bsdf;

struct interaction {
  interaction() : t(0) {}
  interaction(const point3 &p, const vec3 &n, const vec3 &wo, double t,
              const MediumInterface *m)
      : p(p), n(n), wo(wo), t(t), mi(m) {}
  bool isSurfaceInteraction() const { return n != vec3{}; }

  // we ignore the medium for now
  ray spawnRay(const vec3 &d) const {
    return ray{p + n * EPS, d, 0, D_INFINITY, t};
  }
  ray spawnRay(const point3 &d, bool) const {
    point3 x = p + n * EPS;
    return ray{x, d - x, 0, 1 - EPS, t};
  }
  ray spawnRay(const interaction &in) const {
    point3 p1 = p + n * EPS;
    point3 p2 = in.p + in.n * EPS;
    vec3 d = p2 - p1;
    return ray{p1, d, 0, 1 - EPS, t};
  }
  point3 p;
  vec3 wo;
  vec3 n;
  double t;
  const MediumInterface *mi;
};

// this does not have differential geometry since i don't particularly care
// about antialiasing - everything will get blurred anyways
struct SurfaceInteraction : public interaction {
  SurfaceInteraction() {}
  SurfaceInteraction(const point3 &p, const vec3 &n, const vec3 &wo, double t,
                     const MediumInterface *m, const shape *s, const point3 &uv,
                     const vec3 &dpdu, const vec3 &dpdv)
      : interaction(p, n, wo, t, m), s(s), uv(uv), dpdu(dpdu), dpdv(dpdv) {}
  vec3 dpdu;
  vec3 dpdv;
  point3 uv;
  const shape *s = nullptr;
  const primitive *p = nullptr;

  // probably want a unique ptr here lol
  bsdf *b = nullptr;
};
class MediumInteraction : public interaction {};
} // namespace miao
#endif // interaction_hpp
