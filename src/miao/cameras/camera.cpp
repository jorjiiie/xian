#include "miao/cameras/camera.hpp"

namespace miao {

double TempCamera::gen_ray(int x, int y, RNG &rng, ray &r) const {

  double a = rng.rfloat();
  double b = rng.rfloat();
  while (a * a + b * b >= 1) {
    a = rng.rfloat();
    b = rng.rfloat();
  }
  vec3 offset = (a * ap / 2.0 * up) + (b * ap / 2.0 * v);

  vec3 look = this->ll + (v * (x + rng.rfloat()) * d_v) +
              (up * (y + rng.rfloat()) * d_v) - offset;

  r = ray{this->pos + offset, look.unit(), 0};
  return 1;
}

} // namespace miao
