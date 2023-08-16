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
  ProgressiveRenderer(scene &s_, camera &cam_, int epochs_, int spe_ = 16)
      : s(s_), cam(cam_), leunuchs(std::make_shared<Integrator>(&cam_, spe_)),
        epochs(epochs_), spe(spe_) {}
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
