#pragma once
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "miao/cameras/film.hpp"
#include "miao/core/ray.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/transform.hpp"

namespace miao {

// camera holds a medium to describe the medium of the medium
class camera {
public:
  camera(const Transformation &ctw, film &f, double t0, double t1)
      : ctw(ctw), f(f), t0(t0), t1(t1) {}
  virtual double gen_ray(int x, int y, RNG &rng, ray &r) const = 0;
  const Transformation &ctw;
  film &f;
  double t0, t1;
};

class ProjectionCamera : public camera {
public:
  ProjectionCamera(const Transformation &ctw, film &f, double t0, double t1,
                   double apeture, double focd, double screenX, double screenY)
      : camera(ctw, f, t0, t1), apeture(apeture), focd(focd) {}

private:
  double apeture, focd;
};

// default we look at 0,0,1, and any transformations are based off of that
class TempCamera : public camera {
public:
  TempCamera(film &f, const vec3 &pos, const vec3 &up, const vec3 &at,
             double foc, double ap, double fov)

      : camera(Transformation::scale(1, 1, 1), f, 0.0, 0.0), pos(pos), up(up),
        at(at), foc(foc), ap(ap), width(f.get_width()), height(f.get_height()) {

    d = (at - pos).unit();

    v = vec3::cross(d, up);
    d_v = 2.0 * foc * std::tan(fov / 360.0 * PI) / (1.0 * width);

    this->ll = d * foc - width / 2.0 * v * d_v - height / 2.0 * up * d_v;
  }
  virtual double gen_ray(int x, int y, RNG &rng, ray &r) const override;

  vec3 pos;
  vec3 up;
  vec3 at;
  vec3 d;
  vec3 v;
  vec3 ll;
  int width, height;
  double foc, ap;

  double d_v; // the factor that we multiply by
};

} // namespace miao

#endif // CAMERA_HPP
