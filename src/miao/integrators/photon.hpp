#pragma once
#ifndef PHOTON_HPP
#define PHOTON_HPP

#include "miao/integrators/common.hpp"
#include "miao/integrators/integrator.hpp"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace miao {

class SurfaceInteraction;
class MediumInteraction;
class bsdf;
class discrete_1d;

// a photon mapping integrator in volumetric spaces
class PhotonIntegrator : public SampleIntegrator {
public:
  static constexpr int m_depth = 10;
  static constexpr int num_photons = 100000;
  static constexpr double inv_photons = 1.0 / num_photons;
  PhotonIntegrator(camera *cam, int samples = 32, double r = .5)
      : SampleIntegrator(cam, samples), radius(r) {}
  void set_radius(double r) { radius = r; }
  virtual void preprocess(const scene &s) override;
  virtual spectrum estimate_indirect(const SurfaceInteraction &) const;
  virtual spectrum estimate_indirect(const MediumInteraction &) const;
  virtual spectrum Li(const ray &r, const scene &s, RNG &rng,
                      int depth = 0) const override;

private:
  double radius;
  struct cell {
    int i, j, k;
    bool operator==(const cell &c) const {
      return i == c.i && j == c.j && k == c.k;
    }
  };
  struct hash {
    size_t operator()(const cell &c) const {
      return std::hash<int>{}(c.i) ^ std::hash<int>{}(c.j) ^
             std::hash<int>{}(c.k);
    }
  };
  struct Photon {
    vec3 pos;
    vec3 wo;
    spectrum flux;
  };
  cell get_cell(const vec3 &p) const {
    int i = std::floor(p.x / radius);
    int j = std::floor(p.y / radius);
    int k = std::floor(p.z / radius);
    return cell{i, j, k};
  }
  // (surface, volume) photon map
  // we forgo the original caustic vs illumination maps by jensen
  std::unordered_map<cell, std::vector<Photon>, hash> spm, vpm;
  spectrum sample_lights(const scene &s, ray &r, discrete_1d &distr,
                         RNG &rng) const;
  // fnc is a function that calculates the bsdf or phase function wrt a given
  // direction wi
  spectrum
  estimate(const cell &c,
           const std::unordered_map<cell, std::vector<Photon>, hash> &mp,
           const std::function<spectrum(const vec3 &)> &, const vec3 &) const;
};

} // namespace miao
#endif // PHOTON_HPP
