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

class integrator {
public:
  virtual ~integrator();
  virtual void render(const scene &) = 0;
};

// abstract class for integrators that sample
// i think we should take a film here basically.
class SampleIntegrator : public integrator {
public:
  SampleIntegrator(std::shared_ptr<camera> cam, film &f) : cam(cam), f(f) {}
  virtual ~SampleIntegrator();
  virtual spectrum Li(const ray &r, const scene &s, RNG &rng,
                      int depth = 0) const = 0;
  void render(const scene &s) override;

protected:
  std::shared_ptr<camera> cam;
  film &f;
};

class PathIntegrator : public SampleIntegrator {
public:
  const int m_depth = 10;
  PathIntegrator(std::shared_ptr<camera> cam, film &f)
      : SampleIntegrator(cam, f) {}
  virtual spectrum Li(const ray &r, const scene &s, RNG &rng,
                      int depth = 0) const override;
};

} // namespace miao

#endif // INTEGRATOR_HPP
