#include "miao/core/primitive.hpp"
#include "miao/core/interaction.hpp"
#include "miao/core/shape.hpp"

#ifndef RELEASE_MODE
#define DEBUG_ENABLED
#endif
#include "miao/core/debug.hpp"

// animations should be pretty easy by throwing a layer on top of this.
namespace miao {

BBox GeoPrimitive::worldbound() const { return s->worldBBox(); }
std::optional<SurfaceInteraction> GeoPrimitive::intersect(const ray &r,
                                                          double &t) const {
  SurfaceInteraction isect;

  cnter_::prim_tests++;
  if (!s->intersect(r, t, isect))
    return {};
  cnter_::good_tests++;
  isect.pr = this;

  if (mi.transition()) {
    isect.mi = mi;
  } else {
    isect.mi = MediumInterface(mi.in);
  }
  // something about medium blah blah who cares
  return isect;
}

const AreaLight *aggregate::get_area_light() const {
  FAIL("AREA LIGHT CALLED ON AGGREGATE");
  return nullptr;
}
const material *aggregate::get_material() const {
  FAIL("MATERIAL GOT ON AGGREGATE");
  return nullptr;
}

BBox dumb_aggregate::worldbound() const { return wb; }
std::optional<SurfaceInteraction> dumb_aggregate::intersect(const ray &r,
                                                            double &) const {
  std::optional<SurfaceInteraction> ret = {};
  double tt = 69123;
  for (const auto &x : p) {
    double tmp = 0;
    auto y = x.intersect(r, tmp);
    if (y) {
      if (ret) {
        if (tt > tmp)
          ret = std::move(*y), tt = tmp;

      } else {
        ret = std::move(*y);
        tt = tmp;
      }
    }
  }

  return ret;
}

} // namespace miao
