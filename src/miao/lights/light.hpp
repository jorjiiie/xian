#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "miao/core/ray.hpp"
#include "miao/core/spectrum.hpp"

namespace miao {
class light {
public:
  virtual spectrum Le(const ray &r) const;
};

class AreaLight : public light {};
} // namespace miao

#endif // LIGHT_HPP
