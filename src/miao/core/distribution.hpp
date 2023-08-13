#pragma once
#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

// defines a bunch of samplers for distributions

#include <vector>

namespace miao {

class RNG;
class discrete_1d {
public:
  discrete_1d(const std::vector<double> &);
  int sample(RNG &rng) const;

private:
  std::vector<double> weights;
};

} // namespace miao

#endif // DISTRIBUTION_HPP
