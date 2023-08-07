#include "miao/renderers/progressive.hpp"

#include "miao/integrators/integrator.hpp"

namespace miao {
void ProgressiveRenderer::render(const std::function<void()> &callback) {
  // some preprocessing garbage
  for (int i = 0; i < epochs; i++) {
    std::cerr << "currently rendering epoch " << (i + 1) << "\n";
    leunuchs->render(s);
    callback();
  }
}
} // namespace miao
