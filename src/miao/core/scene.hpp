#pragma once
#include "miao/core/shape.hpp"
#ifndef SCENE_HPP
#define SCENE_HPP

#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "miao/core/debug.hpp"
#include "miao/core/interaction.hpp"
#include "miao/core/primitive.hpp"

namespace miao {

class aggregate;
class TriangleMesh;
class light;
class scene {
public:
  scene(const std::vector<std::shared_ptr<light>> &lights,
        std::shared_ptr<aggregate> agg)
      : lights(lights), agg(agg) {}
  scene(const std::string &s) { load_from_obj(s); }
  std::optional<SurfaceInteraction> intersect(const ray &r, double t) const {
    cnter_::rays_cast++;
    return agg->intersect(r, t);
  }
  std::optional<SurfaceInteraction> intersectTr(const ray &r, double t,
                                                spectrum &tr, RNG &rng) const;

  bool load_from_obj(const std::string &fp);
  std::vector<std::shared_ptr<light>> lights;
  std::shared_ptr<aggregate> agg;

private:
  std::unique_ptr<TriangleMesh> triangles;
};

class visibility {
public:
  visibility() {}
  visibility(const interaction &p0_, const interaction &p1_)
      : p0(p0_), p1(p1_) {}
  bool visible(const scene &s) const;
  // calculates throughput (generalization of visibile)
  spectrum tr(const scene &s, RNG &rng) const;
  interaction p0, p1;
};

} // namespace miao
#endif // scene_hpp
