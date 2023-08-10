#include "shape.hpp"

namespace miao {

vec3 uniform_sphere(RNG &rng) {
  double z = 1 - 2 * rng.rfloat();
  double r = std::sqrt(max(0.0, 1 - z * z));
  double phi = 2 * PI * rng.rfloat();
  return vec3{r * std::cos(phi), r * std::sin(phi), z};
}
inline double uniform_sphere_pdf() { return INV_PI / 4; }

BBox sphere::getBBox() const {
  return BBox{vec3{-radius, -radius, -radius}, vec3{radius, radius, radius}};
}

// t is outgoing time
bool sphere::intersect(const ray &r, double &t,
                       SurfaceInteraction &isect) const {
  double phi, theta;
  point3 hit;
  vec3 oc = r.o - origin;

  double a = r.d.msq();
  double b = 2 * vec3::dot(oc, r.d);
  double c = oc.msq() - radius * radius;
  double t0, t1;
  if (!quadratic(a, b, c, t0, t1))
    return false;
  if (t0 > r.maxt || t1 <= 0)
    return false;

  double T = t0;
  if (T < 0) {
    T = t1;
    if (T > r.maxt)
      return false;
  }
  vec3 hp = r.o + T * r.d;
  isect.p = hp;
  isect.wo = r.d;
  isect.n = (isect.p - origin) / radius;
  isect.sn = isect.n;

  isect.t = T;

  t = T;
  return true;

  // for now we just don't care about the parametric stuff
}
double sphere::area() const {
  // compute based off of bounds!
  // integral of 1 from tmin to tmax, 0 to phimax sinT dtdp
  return 4 * PI * radius; // naive no bounds
}
interaction sphere::sample(RNG &rng) const {
  vec3 v = uniform_sphere(rng);
  point3 point = radius * v;
  SurfaceInteraction it;
  it.n = v;
  it.p = point + origin;
  return it;
}
interaction sphere::sample(const interaction &, RNG &r) const {
  return sample(r);
}
double sphere::pdf(const interaction &r, const vec3 &) const {
  return shape::pdf(r);
}
} // namespace miao
