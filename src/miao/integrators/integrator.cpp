#include "integrator.hpp"

#include "miao/cameras/film.hpp"

#include "miao/core/interaction.hpp"
#include "miao/core/material.hpp"
#include "miao/core/primitive.hpp"
#include "miao/core/scene.hpp"
#include "miao/core/utils.hpp"

#include "miao/lights/light.hpp"

#include <iomanip>
#include <omp.h>
#include <vector>

namespace miao {
void SampleIntegrator::render(const scene &s) {

  int block_size = 16;
  int width;
  int height;
  int samples;

  int num_threads = 1;
  std::vector<pcg32> rngs(num_threads);

  auto trace = [&](int x, int y) {
    int sx = 16 * x;
    int sy = 16 * y;
    int ex = min(16 * (x + 1), width);
    int ey = min(16 * (y + 1), height);
    RNG &rng = rngs[0];
    for (int i = sx; i < ex; i++) {
      for (int j = sy; j < ey; j++) {
        spectrum L{};

        for (int k = 0; k < samples; k++) {
          ray r;

          L += Li(r, s, rng, 10);
        }
        // do something with L (add to film or sumthin)

        L /= samples;
        f.add_sample(i, j, L);
        // f->add(L) or something
      }
    }
  };

  int nx;
  int ny;
  int cnt = 0;
  std::cerr << std::fixed << std::setprecision(3);

#pragma omp parallel for collapse(2)
  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < ny; j++) {
      trace(i, j);
      ++cnt;
      std::cerr << "\rRendering progress: " << cnt << "/" << nx * ny
                << " tiles rendered (" << cnt * 100.0 / nx / ny << "%)    ";
    }
  }
}

spectrum PathIntegrator::Li(const ray &ra, const scene &s, RNG &rng,
                            int depth) const {
  spectrum L{};
  spectrum throughput{1.0};
  bool lastSpecular = true;
  ray r = ra;
  for (int i = 0; i < m_depth; i++) {
    auto y = s.intersect(r, 0);
    if (!y) { // bleak blackness of the screen D:
      break;
    }
    SurfaceInteraction &isect = *y;

    const AreaLight *alight = isect.pr->get_area_light();
    L += throughput * alight->Le(r);

    bsdf *b = isect.b;
    vec3 wo = r.d, wi;
    double pdf;
    spectrum f = b->sample_f(wo, wi, isect.n, rng, pdf);
    if (f.isBlack() || pdf == 0.0)
      break;
    throughput *= f * std::abs(vec3::dot(isect.n, wi)) / pdf;

    r.o = isect.p + EPS * isect.n;
    r.d = wi;
    if (i > 3) {
      double q = std::max(0.05, 1 - throughput.magnitude());
      if (rng.rfloat() < q)
        break;
      throughput /= (1 - q);
    }
  }
  return L;
}
} // namespace miao
