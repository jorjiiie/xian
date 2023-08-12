#pragma once
#ifndef VOLUME_INTEGRATOR_HPP
#define VOLUME_INTEGRATOR_HPP

#include "miao/integrators/common.hpp"
#include "miao/integrators/integrator.hpp"

namespace miao {
class VolumeIntegrator : public SampleIntegrator {
public:
  VolumeIntegrator(camera *cam, int samples = 32, int depth = 30)
      : SampleIntegrator(cam, samples), m_depth(depth) {}
  virtual spectrum Li(const ray &r, const scene &s, RNG &rng,
                      int depth = 0) const override;
  const int m_depth;
};
} // namespace miao

#endif // VOLUME_INTEGRATOR_HPP
