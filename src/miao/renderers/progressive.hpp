#pragma once
#ifndef PROGRESSIVE_HPP
#define PROGRESSIVE_HPP

#include <functional>
#include <memory>

#include "miao/integrators/integrator.hpp"
namespace miao {

class scene;
class camera;
class ProgressiveRenderer {

public:
  ProgressiveRenderer(scene &s, camera &cam, int epochs, int spe = 16)
      : s(s), cam(cam), leunuchs(std::make_shared<PathIntegrator>(&cam, spe)),
        epochs(epochs), spe(spe) {}
  void render(const std::function<void(int)> &callback);

private:
  scene &s;
  camera &cam;
  std::shared_ptr<integrator> leunuchs;
  int epochs;
  int spe;
};
} // namespace miao
#endif
