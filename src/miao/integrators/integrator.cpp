#include "miao/integrators/integrator.hpp"
#include "miao/integrators/common.hpp"

#include "miao/cameras/camera.hpp"
#include "miao/cameras/film.hpp"

#include "miao/core/interaction.hpp"
#include "miao/core/material.hpp"
#include "miao/core/primitive.hpp"
#include "miao/core/scene.hpp"
#include "miao/core/utils.hpp"

#include "miao/lights/light.hpp"

#include "miao/core/debug.hpp"

#include <iomanip>
#include <memory>
#include <omp.h>
#include <vector>

namespace miao {

void SampleIntegrator::render(const scene &s) {

  int tile_size = 32;
  int width = cam->f.get_width();
  int height = cam->f.get_height();

  int num_threads = 1;

// if openmp enabled
#if defined(_OPENMP) && !defined(DEBUG_ENABLED)
  num_threads = std::max(1, omp_get_max_threads());
#endif

  omp_set_num_threads(num_threads);
  std::cerr << "USING " << num_threads << " THREADS\n";
  std::vector<pcg32> rngs(num_threads);
  for (int i = 0; i < num_threads; i++) {
    rngs[i].seed(rand() * rand(), rand() * rand());
  }

  auto trace = [&](int x, int y) {
    int sx = tile_size * x;
    int sy = tile_size * y;
    int ex = min(tile_size * (x + 1), width);
    int ey = min(tile_size * (y + 1), height);
    int idx = 0;

#ifdef _OPENMP
    idx = omp_get_thread_num(); // DONUT SHARE RNGS D:
#endif

    RNG &rng = rngs[idx];
    for (int i = sx; i < ex; i++) {
      for (int j = sy; j < ey; j++) {
        spectrum L{};

        for (int k = 0; k < samples; k++) {
          ray r;
          cam->gen_ray(i, j, rng, r);
          L += Li(r, s, rng, 10);
        }
        // do something with L (add to film or sumthin)

        cam->f.add_sample(i, j, L, samples);
        // f->add(L) or something
      }
    }
  };

  int nx = (width - 1) / tile_size + 1;
  int ny = (height - 1) / tile_size + 1;
  int cnt = 0;
  std::cerr << std::fixed << std::setprecision(3);
#pragma omp parallel for schedule(dynamic) collapse(2)
  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < ny; j++) {
      trace(i, j);
      ++cnt;
      std::cerr << "\rRendering progress: " << cnt << "/" << nx * ny
                << " tiles rendered (" << cnt * 100.0 / nx / ny << "%)    ";
    }
  }

  std::cerr << "\n";
}

spectrum PathIntegrator::Li(const ray &ra, const scene &s, RNG &rng,
                            int depth) const {
  spectrum L{};
  spectrum throughput{1.0};
  bool lastSpecular = false;
  ray r = ra;
  int i;
  for (i = 0; i < m_depth; i++) {
    auto y = s.intersect(r, 0);
    if (!y) { // bleak blackness of the screen D:
      break;
    }
    SurfaceInteraction &isect = *y;

    if (i == 0 || lastSpecular) {
      const AreaLight *alight = isect.pr->get_area_light();
      if (alight) {
        L += throughput * alight->Le(r);
        // if (onoff)
        // DEBUG("hit light after! THIS IS A L(DS)*SDE PATH", throughput.ts());
      }
    }
    if (i != 0) {
      L += throughput * sample_light(isect, s, rng);
      DEBUG("FRA\n");
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

    if (f.isBlack() || pdf == 0.0)
      break;
    throughput *= f * std::abs(vec3::dot(isect.n, wi)) / pdf;

    r.o = isect.p;
    r.d = wi;
    if (i > 3) {
      double q = std::max(0.05, 1 - throughput.magnitude() * 0.3333);
      if (rng.rfloat() < q)
        break;
      throughput /= (1 - q);
    }
  }
  return L;
}
} // namespace miao
