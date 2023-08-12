#include "miao/integrators/volumeintegrator.hpp"

#include "miao/cameras/camera.hpp"
#include "miao/cameras/film.hpp"

#include "miao/core/interaction.hpp"
#include "miao/core/material.hpp"
#include "miao/core/primitive.hpp"
#include "miao/core/scene.hpp"
#include "miao/core/utils.hpp"

#include "miao/lights/light.hpp"

#include "miao/core/debug.hpp"

#include "miao/volumes/base.hpp"
#include "miao/volumes/medium.hpp"

namespace miao {
spectrum VolumeIntegrator::Li(const ray &rx, const scene &s, RNG &rng,
                              int depth) const {
  spectrum L{};
  spectrum tp{1.0};
  bool lastSpecular = false;

  bool none = false;
  ray r = rx;
  int i;
  for (i = 0; i < m_depth; i++) {
    // find intersection?
    auto y = s.intersect(r, 0);
    SurfaceInteraction &isect = *y;
    if (!y)
      isect.t = D_INFINITY;
    MediumInteraction mi;
    // const medium *med = isect.get_medium(r.d);
    const medium *med = r.m;
    r.maxt = isect.t;
    if (med != nullptr) {
      tp *= med->sample(r, rng, mi); // it goes that far anyways?
      DEBUG("YES MEDIA!");
    } else {
      DEBUG("NO MEDIA!");
      none = true;
      if (!y)
        break;
    }
    if (none) {
      DEBUG("FUCK ", r.o.ts(), r.d.ts());
    }

    if (mi.ph != nullptr) { // if medium interaction
      // must modify sample_light to account for media
      L += tp * sample_light(mi, s, rng);
      // sample the phase function here
      // update the ray (don't do this in heterogeneous media or non const phase
      // functions LOL)
      mi.ph->sample_p(r.d, r.d, rng);

      DEBUG("what is going on");
      r.o = mi.p;
      r.d = r.d;
    } else {
      // surface interaction copy paste blah blah
      if (i == 0 || lastSpecular) {
        const AreaLight *alight = isect.pr->get_area_light();
        if (alight) {
          L += tp * alight->Le(r);
        }
      }
      if (i != 0) {
        L += tp * sample_light(isect, s, rng);
      }

      auto mat = isect.pr->get_material();
      if (mat == nullptr) {
        i--;
        r.o = isect.p;
        continue;
      }

      auto b = mat->get_scatter(isect);
      vec3 wo = r.d, wi;
      double pdf;
      bxdf_t sampled = BSDF_NONE;
      spectrum f = b->sample_f(wo, wi, isect.n, rng, pdf, BSDF_ALL, sampled);

      lastSpecular = (sampled & BSDF_SPECULAR) != 0;
      if (lastSpecular) {
        DEBUG("SURFACE HIT ON GLASS", isect.mi.transition());
      }

      if (isect.mi.transition())
        r.m = isect.get_medium(wi);

      if (f.isBlack() || pdf == 0.0)
        break;
      tp *= f * std::abs(vec3::dot(isect.n, wi)) / pdf;

      r.o = isect.p;
      r.d = wi;
    }

    r.maxt = D_INFINITY;
    if (i > 3) {
      double q = std::max(0.05, 1 - tp.magnitude() * 0.3333);
      if (rng.rfloat() < q)
        break;
      tp /= (1 - q);
    }
  }
  return L;
}

} // namespace miao
