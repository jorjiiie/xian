#pragma once
#ifndef SCENE_HPP
#define SCENE_HPP

#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "miao/core/debug.hpp"
#include "miao/core/interaction.hpp"
#include "miao/core/primitive.hpp"
#include "miao/core/shape.hpp"
#include "miao/core/transform.hpp"

namespace miao {

class aggregate;
class light;
class scene {
public:
  scene(const std::vector<std::shared_ptr<light>> &lights,
        std::shared_ptr<aggregate> agg)
      : lights(lights), agg(agg) {}
  scene(const std::string &s) { load_from_obj(s); }
  scene() {}
  std::optional<SurfaceInteraction> intersect(const ray &r, double t) const {
    cnter_::rays_cast++;
    return agg->intersect(r, t);
  }
  void add_prim(std::shared_ptr<primitive> p) { prims.push_back(p); }
  std::optional<SurfaceInteraction> intersectTr(const ray &r, double t,
                                                spectrum &tr, RNG &rng) const;

  bool load_from_obj(const std::string &fp,
                     const Transformation &t = Transformation::identity);
  // builds the scene from all the prims!
  bool build();
  std::vector<std::shared_ptr<light>> lights;
  std::shared_ptr<aggregate> agg;

private:
  bool loaded = false;
  std::vector<std::shared_ptr<primitive>> prims;
  std::vector<std::unique_ptr<TriangleMesh>> triangles;
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
