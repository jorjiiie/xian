#include "miao/integrators/common.hpp"

#include "miao/core/interaction.hpp"
#include "miao/core/material.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/scene.hpp"
#include "miao/core/shape.hpp"
#include "miao/lights/light.hpp"

namespace miao {
spectrum direct(const interaction &it, const light &yo, const scene &s,
                RNG &rng) {
  vec3 wi;
  double lpdf = 0, spdf = 0;
  spectrum Ld{};
  const SurfaceInteraction &isect = (const SurfaceInteraction &)it;
  visibility v;
  spectrum li = yo.Li(it, rng, wi, lpdf, &v);
  // test visibility
  bxdf_t sampled;
  auto b = isect.pr->get_material()->get_scatter(isect);
  if (lpdf > 0 && !li.isBlack()) {

    spectrum tp{1.0};
    if (it.isSurfaceInteraction()) {
      tp = b->f(wi, isect.wo, BSDF_ALL) *
           std::abs(vec3::dot(wi, isect.n)); // should be isect.sn but oh well
      spdf = b->pdf(wi, isect.wo, isect.n);  // same here
    }
    DEBUG("JASKDLJASKLDJAKSLDJASDLKJAKSD");
    if (!tp.isBlack()) {
      double weight = bh(1, lpdf, 1, spdf);
      /* DEBUG("we have something here", tp.ts(), " ", li.ts(), " ", weight, "
       * ", */
      /*       lpdf, " ", spdf); */
      if (v.visible(s)) {
        Ld += tp * li * weight / lpdf;
        DEBUG("VISIBLE");
      }
      // else

      // DEBUG("not visible!");
    }
  }

  // DEBUG("light from light sampling,", Ld.ts());

  {
    spectrum tp;
    // shading normals blah blah
    tp = b->sample_f(isect.wo, wi, isect.n, rng, spdf, BSDF_ALL, sampled);
    tp *= std::abs(vec3::dot(wi, isect.n));
    // DEBUG("stuff ", wi.ts(), " ", spdf);
    if (spdf > 0 && !tp.isBlack()) {
      double weight = 1; // if its specular then its zero D:  need to be able to
                         // test for this
      // check light pdf
      // lpdf = yo.pdf() ?? why is this necessary (for MIS D:)
      if (!(sampled & BSDF_SPECULAR)) {
        lpdf = yo.pdf_li(isect, wi);
        if (lpdf == 0)
          return Ld;
        weight = bh(1, spdf, 1, lpdf);
      }

      // DEBUG("NONZERO\n");
      ray r{isect.p, wi, 0};
      auto in = s.intersect(r, 0);
      if (in && in->pr->get_area_light() == &yo) {
        // this is the light we want
        // DEBUG("AJSKDLAJSKLDJASLKDJKLASD");
        Ld += tp * yo.Le(ray{in->p, -wi, 0}) * weight / spdf;
      }
    }
  }

  return Ld;
}

spectrum sample_light(const interaction &it, const scene &s, RNG &rng) {
  int n = s.lights.size();
  if (n == 0)
    return spectrum{0.0};
  int sampled = (int)(rng.rfloat() * n);
  const light &y = *s.lights[sampled];

  auto spec = direct(it, y, s, rng) * n;

  DEBUG("HIII ", sampled, " ", spec.ts());

  return spec;
}

double bh(int lsamp, double lpdf, int ssamp, double spdf) {
  return (lsamp * lpdf) / (lsamp * lpdf + ssamp * spdf);
}

} // namespace miao
