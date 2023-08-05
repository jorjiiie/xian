#include "shape.hpp"

namespace miao {
BBox sphere::getBBox() const {
  return BBox{vec3{-radius, -radius, -radius}, vec3{radius, radius, radius}};
}

// t is outgoing time
bool sphere::intersect(const ray &r, double &t,
                       SurfaceInteraction &isect) const {
  double phi, theta;
  point3 hit;
  ray ry = (*wto)(r);

  double a = vec3::dot(ry.d, ry.d);
  double b = 2 * vec3::dot(ry.o, ry.d);
  double c = vec3::dot(ry.o, ry.o) - radius;
  double t0, t1;
  if (!quadratic(a, b, c, t0, t1))
    return false;
  if (t0 > ry.maxt || t1 <= 0)
    return false;

  double T = t0;
  if (T < 0) {
    T = t1;
    if (T > ry.maxt)
      return false;
  }

  t = T;
  return true;

  // for now we just don't care about the parametric stuff

  // adjust hit and phi based off of the t givem. this is to
  auto adjustHit = [&](double d) {
    hit = ry.o + d * ry.d;

    phi = std::atan2(hit.y, hit.x);
    if (phi < 0)
      phi += 2 * PI;
  };
  adjustHit(T);

  return true;
}
double sphere::area() const {
  // compute based off of bounds!
  // integral of 1 from tmin to tmax, 0 to phimax sinT dtdp
  return 4 * PI * radius; // naive no bounds
}
interaction sphere::sample(RNG &r) const {}
interaction sphere::sample(const interaction &i, RNG &r) const {}
double sphere::pdf(const interaction &r, const vec3 &wi) const {}
} // namespace miao
