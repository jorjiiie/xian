#pragma once
#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include <memory>

#include "miao/core/ray.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/spectrum.hpp"

namespace miao {

class film;
class scene;
class camera;
class interaction;

class integrator {
public:
  virtual void preprocess(const scene &s) {}
  virtual void render(const scene &) = 0;
};

// abstract class for integrators that sample
// i think we should take a film here basically.
class SampleIntegrator : public integrator {
public:
  SampleIntegrator(camera *cam, int samples = 32)
      : cam(cam), samples(samples) {}
  virtual spectrum Li(const ray &r, const scene &s, RNG &rng,
                      int depth = 0) const = 0;
  virtual void render(const scene &s) override;

protected:
  camera *cam;
  int samples;
};

class PathIntegrator : public SampleIntegrator {
public:
  const int m_depth = 10;
  PathIntegrator(camera *cam, int samples = 32)
      : SampleIntegrator(cam, samples) {}
  virtual spectrum Li(const ray &r, const scene &s, RNG &rng,
                      int depth = 0) const override;
};

} // namespace miao

#endif // INTEGRATOR_HPP
