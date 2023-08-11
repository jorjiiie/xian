#include "miao/core/scene.hpp"

namespace miao {
bool visibility::visible(const scene &s) const {
  vec3 d = (p1.p - p0.p);
  double maxt = d.magnitude();
  ray r{p0.p + (d / maxt * EPS), d / maxt, 0};

  cnter_::rays_cast++;

  auto y = s.intersect(r, 0);
  if (y) {
    return !(y->t + EPS < maxt);
  }
  FAIL("WTF");
}

} // namespace miao
