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
class PhaseFunction;
// a photon mapping integrator in volumetric spaces
class PhotonIntegrator : public SampleIntegrator {
public:
  static constexpr int m_depth = 15;
  static constexpr double default_dist =
      0.3; // this should be larger than the radius!
  static constexpr int max_beam_steps =
      5; // 2^max_beam_steps is the actual amount
  static constexpr double big_delta = 0.3;
  static constexpr double max_dist = default_dist * 100;
  PhotonIntegrator(camera *cam, int samples = 32, double r = .05,
                   int np = 300000)
      : SampleIntegrator(cam, samples), num_photons(np), inv_photons(1.0 / np),
        v_radius(r), s_radius(r) {}
  virtual void preprocess(const scene &s) override;
  virtual spectrum estimate_indirect(const SurfaceInteraction &) const;
  virtual spectrum estimate_indirect(const MediumInteraction &) const;
  virtual spectrum estimate_indirect_v(const ray &,
                                       const PhaseFunction &) const;
  // calculates incoming beam light (returned), and then returns the overall
  // transmittance for the beam in the spectrum passed in
  // used for calculating volumetric photon estimates.
  // (uses a biased estimate)
  // for a homogenous medium?
  virtual spectrum estimate_beam(const ray &, double, const medium *,
                                 spectrum &, RNG &) const;
  virtual spectrum Li(const ray &r, const scene &s, RNG &rng,
                      int depth = 0) const override;

protected:
  spectrum estimate_steps(const ray &, double, const medium *, spectrum &,
                          RNG &, spectrum &, int depth = 0) const;
  int num_photons;
  double inv_photons;
  double v_radius, s_radius;
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
  cell get_cell_s(const vec3 &p) const {
    int i = std::floor(p.x / s_radius + 0.5);
    int j = std::floor(p.y / s_radius + 0.5);
    int k = std::floor(p.z / s_radius + 0.5);
    return cell{i, j, k};
  }
  cell get_cell_v(const vec3 &p) const {
    int i = std::floor(p.x / v_radius + 0.5);
    int j = std::floor(p.y / v_radius + 0.5);
    int k = std::floor(p.z / v_radius + 0.5);
    return cell{i, j, k};
  }
  // (surface, volume) photon map
  // we forgo the original caustic vs illumination maps by jensen
  std::unordered_map<cell, std::vector<Photon>, hash> spm, vpm;
  spectrum sample_lights(const scene &s, ray &r, discrete_1d &distr, RNG &rng,
                         spectrum) const;
  // fnc is a function that calculates the bsdf or phase function wrt a given
  // direction wi
  spectrum
  estimate(const cell &c,
           const std::unordered_map<cell, std::vector<Photon>, hash> &mp,
           const std::function<spectrum(const vec3 &)> &, const vec3 &,
           double radius) const;
};

} // namespace miao
#endif // PHOTON_HPP
