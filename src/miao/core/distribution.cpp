#include "miao/core/distribution.hpp"

#include "miao/core/rng.hpp"

namespace miao {
discrete_1d::discrete_1d(const std::vector<double> &v) {
  if (v.size() == 0)
    return;
  weights.resize(v.size());
  weights[0] = v[0];
  double s = v[0];
  for (size_t i = 1; i < v.size(); i++) {
    weights[i] = weights[i - 1] + v[i];
    s += v[i];
  }
  for (double &k : weights)
    k /= s;
  // weights is now a bunch of small itty bitty intervals!
}

int discrete_1d::sample(RNG &rng) const {
  int l = 0, r = weights.size();
  int idx = -1;
  double xi = rng.rfloat();
  while (l <= r) {
    int m = (l + r) / 2;
    if (weights[m] > xi) {
      // this is good but must bring the right over
      idx = m;
      r = m - 1;
    } else {
      l = m + 1;
    }
  }
  return idx;
}
} // namespace miao
