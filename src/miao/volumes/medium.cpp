#include "miao/volumes/medium.hpp"

#include "miao/core/interaction.hpp"
#include "miao/core/utils.hpp"
#include "miao/volumes/base.hpp"

#include "miao/core/debug.hpp"

#include <memory>
namespace miao {
spectrum homogeneous::sample(const ray &r, RNG &rng,
                             MediumInteraction &mi) const {
  int id = rng.rint() % spectrum::N;
  double t = -std::log(1 - rng.rfloat()) / st[id];

  bool media = t < r.maxt;
  if (t < r.maxt) {
    // init phase function, mi is valid
    mi.p = r.o + t * r.d;
    mi.wo = r.d;
    mi.t = t;
    mi.ph = std::make_shared<isotropic>();
  } else {
  }
  spectrum tr = homogeneous::tr(r, std::min(t, D_INFINITY - 1), rng);
  spectrum pdf_component = media ? (st * tr) : tr;
  double weight = 0;
  for (int i = 0; i < spectrum::N; i++) {
    weight += pdf_component[i];
  }
  weight /= spectrum::N;
  return media ? (tr * ss / weight) : tr / weight;
}

} // namespace miao
