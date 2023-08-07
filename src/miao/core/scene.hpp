#pragma once
#ifndef SCENE_HPP
#define SCENE_HPP

#include <memory>
#include <optional>
#include <vector>

#include "miao/core/interaction.hpp"
#include "miao/core/primitive.hpp"

namespace miao {

class aggregate;
class light;
class scene {
public:
  scene(const std::vector<std::shared_ptr<light>> &lights,
        std::shared_ptr<aggregate> agg)
      : lights(lights), agg(agg) {}
  std::optional<SurfaceInteraction> intersect(const ray &r, double t) const {
    return agg->intersect(r, t);
  }

private:
  std::vector<std::shared_ptr<light>> lights;
  std::shared_ptr<aggregate> agg;
};

} // namespace miao
#endif // scene_hpp
