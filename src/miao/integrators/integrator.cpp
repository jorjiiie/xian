#include "integrator.hpp"

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
    for (int i = sx; i < ex; i++) {
      for (int j = sy; j < ey; j++) {
        spectrum L{};
        for (int k = 0; k < samples; k++) {
          ray r;

          L += Li(r, s, rngs[0], 10);
        }
        // do something with L (add to film or sumthin)
        L /= samples;
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
} // namespace miao
