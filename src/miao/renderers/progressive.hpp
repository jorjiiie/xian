#pragma once
#ifndef PROGRESSIVE_HPP
#define PROGRESSIVE_HPP

#include <functional>
#include <memory>

#include "miao/integrators/integrator.hpp"
#include "miao/integrators/volumeintegrator.hpp"
namespace miao {

class scene;
class camera;

template <typename Integrator> class ProgressiveRenderer {

public:
  ProgressiveRenderer(scene &s, camera &cam, int epochs, int spe = 16)
      : s(s), cam(cam), leunuchs(std::make_shared<Integrator>(&cam, spe)),
        epochs(epochs), spe(spe) {}
  void render(const std::function<void(int)> &callback) {

    for (int i = 0; i < epochs; i++) {
      std::cerr << "currently rendering epoch " << (i + 1) << "\n";
      leunuchs->render(s);
      callback(i);
    }
  }

private:
  scene &s;
  camera &cam;
  std::shared_ptr<Integrator> leunuchs;
  int epochs;
  int spe;
};
} // namespace miao
#endif
