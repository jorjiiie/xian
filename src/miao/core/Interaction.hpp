#pragma once
#ifndef INTERACTION_HPP
#define INTERACTION_HPP

#include "ray.hpp"
#include "utils.hpp"
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
                     const MediumInterface *m, const vec3 &sn_, double u_,
                     double v_)
      : interaction(p, n, wo, t, m), sn(sn_), u(u_), v(v_) {}
  bsdf *get_bsdf(const ray &r);
  vec3 sn;
  double u, v;
  const primitive *pr = nullptr;
  // probably want a unique ptr here lol
  bsdf *b = nullptr;
};
class MediumInteraction : public interaction {};
} // namespace miao
#endif // interaction_hpp
