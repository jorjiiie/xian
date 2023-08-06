#pragma once
#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include <memory>

#include "miao/core/ray.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/spectrum.hpp"

namespace miao {

class scene;
class camera;

class integrator {
public:
  virtual ~integrator();
  virtual void render(const scene &) = 0;
};

// abstract class for integrators that sample
class SampleIntegrator : public integrator {
public:
  SampleIntegrator(std::shared_ptr<camera> cam) : cam(cam) {}
  virtual ~SampleIntegrator();
  virtual spectrum Li(const ray &r, const scene &s, RNG &rng,
                      int depth = 0) const = 0;
  void render(const scene &s) override;

protected:
  std::shared_ptr<camera> cam;
};

class PathIntegrator : public SampleIntegrator {
public:
  PathIntegrator(std::shared_ptr<camera> cam) : SampleIntegrator(cam) {}
  virtual spectrum Li(const ray &r, const scene &s, RNG &rng,
                      int depth = 0) const override;
};

} // namespace miao

#endif // INTEGRATOR_HPP
