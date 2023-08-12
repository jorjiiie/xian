#include "miao/core/scene.hpp"
#include "miao/core/interaction.hpp"
#include "miao/core/utils.hpp"

namespace miao {

std::optional<SurfaceInteraction>
scene::intersectTr(const ray &r, double t, spectrum &tr, RNG &rng) const {
  tr = spectrum{1.0};

  ray x = r;
  for (int i = 0; i < 100; i++) {
    auto y = intersect(x, t);
    if (!y)
      return {}; // this is only because of the lack of infinite lights and
                 // environment maps
    // return the transmittance
    if (x.m != nullptr)
      tr *= x.m->tr(x, y->t, rng);

    if (y->pr->get_material() != nullptr)
      return y;

    x.m = y->get_medium(x.d);
    DEBUG("WHAT\n");
  }
  return {};
}

bool visibility::visible(const scene &s) const {
  vec3 d = (p1.p - p0.p);
  double maxt = d.magnitude();
  ray r{p0.p + (d / maxt * EPS), d / maxt, 0};

  cnter_::rays_cast++;

  auto y = s.intersect(r, 0);
  if (y) {
    return !(y->t + EPS < maxt);
  }
  return true;
}

spectrum visibility::tr(const scene &s, RNG &rng) const {
  vec3 d = (p1.p - p0.p);
  double maxt = d.magnitude();
  ray r{p0.p + (d / maxt * EPS), d / maxt, 0};
  cnter_::rays_cast++;
  spectrum tr{1.0};

  while (true) {
    auto y = s.intersect(r, 0);
    if (!y) {
      if (r.m)
        return r.m->tr(r, D_INFINITY - 1, rng);
      return tr;
    }

    if (y && y->pr->get_material() != nullptr) {
      if (y->t + vec3::eps > maxt)
        return tr;
      return spectrum{0};
    }
    r.maxt = y->t;
    if (r.m != nullptr) {
      tr *= r.m->tr(r, y->t, rng);
    }
    r.maxt = D_INFINITY;

    // continue;
    r.o = r.o + r.d * y->t;
    r.m = y->get_medium(r.d);
    DEBUG("HI", r.o.ts(), " ", y->t, " ", r.d.ts(), " ");
  }
}

} // namespace miao
