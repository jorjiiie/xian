#include "miao/integrators/photon.hpp"

#include "miao/cameras/camera.hpp"
#include "miao/cameras/film.hpp"

#include "miao/core/interaction.hpp"
#include "miao/core/material.hpp"
#include "miao/core/primitive.hpp"
#include "miao/core/scene.hpp"
#include "miao/core/spectrum.hpp"
#include "miao/core/utils.hpp"

#include "miao/lights/light.hpp"

#include "miao/core/debug.hpp"

#include "miao/volumes/base.hpp"
#include "miao/volumes/medium.hpp"

#include <omp.h>

namespace miao {

void PhotonIntegrator::preprocess(const scene &s) {
  // builds the photon map.

  int num_shooters = 1;
#ifdef _OPENMP
  num_shooters = omp_get_num_threads();
#endif

  std::vector<pcg32> rngs(num_shooters);

  int current_photons = 0;
  auto shoot = [&]() {
    int idx = 0;
#ifdef _OPENMP
    idx = omp_get_thread_num();
#endif
    RNG &rng = rngs[idx];

    while (current_photons < num_photons) {
      // sample the scene lights;
      ray r;
      spectrum tp{1.0};
      // we only sample indirect illumination
      bool interacted = false;
      for (int bounces = 0; bounces < m_depth; bounces++) {
        auto tmp = s.intersect(r, 0);
        SurfaceInteraction &isect = *tmp;
        if (!tmp) {
          isect.t = D_INFINITY;
        }
        MediumInteraction mi;

        const medium *med = r.m;
        r.maxt = isect.t;
        if (med != nullptr) {
          tp *= med->sample(r, rng, mi);
        } else {
          // no medium, off to infinity to death
          if (!tmp)
            break;
        }

        if (mi.ph != nullptr) {
          // phase function! get new direction
          vec3 wi;
          if (interacted) {
            int x = std::floor(mi.p.x);
            int y = std::floor(mi.p.y);
            int z = std::floor(mi.p.z);
            cell c{x, y, z};
#pragma omp critical
            {
              current_photons++;
              // this needs to account for the fraction that gets boosted?
              // sigma_s
              vpm[c].push_back(Photon{mi.p, mi.wo, tp});
            }
          }
          // deposit something here btw
          mi.ph->sample_p(r.d, wi, rng);
          interacted = true;
          r.o = mi.p;
          r.d = wi;
        } else {
          // surface interaction!
          auto mat = isect.pr->get_material();
          if (mat == nullptr) {
            bounces--;
            r.o = isect.p;
            continue;
          }
          auto b = mat->get_scatter(isect);
          if ((b->get_flags() & BSDF_DIFFUSE) != 0 && interacted) {
            // deposit a photon here!
            int x = std::floor(isect.p.x);
            int y = std::floor(isect.p.y);
            int z = std::floor(isect.p.z);
            cell c{x, y, z};
#pragma omp critical
            {
              current_photons++;
              spm[c].push_back({Photon{isect.p, isect.wo, tp}});
            }
          }
          interacted = true;
          vec3 wo = r.d, wi;
          double pdf;
          bxdf_t sampled = BSDF_NONE;
          spectrum f =
              b->sample_f(wo, wi, isect.n, rng, pdf, BSDF_ALL, sampled);

          if (isect.mi.transition())
            r.m = isect.get_medium(wi);

          if (f.isBlack() || pdf == 0.0)
            break;
          tp *= f * std::abs(vec3::dot(isect.n, wi)) / pdf;
          r.o = isect.p;
          r.d = wi;
        }
        r.maxt = D_INFINITY;
        if (bounces > 3) {
          double q = std::max(0.05, 1 - tp.magnitude() * 0.3333);
          if (rng.rfloat() < q)
            break;
          tp /= (1 - q);
        }
      }
    }
  };

#pragma omp parallel for
  for (int i = 0; i < num_shooters; i++)
    shoot();

  // oof
}

spectrum PhotonIntegrator::estimate(
    const cell &c, const std::unordered_map<cell, std::vector<Photon>> &mp,
    const std::function<spectrum(const vec3 &)> &fnc, const vec3 &p) const {
  spectrum L{};

  const int &x = c.i;
  const int &y = c.j;
  const int &z = c.k;
  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dx <= 1; dx++) {
      for (int dz = -1; dz <= 1; dz++) {
        cell current{x + dx, y + dy, z + dz};
        if (spm.find(current) != mp.end()) {
          // go through photons
          for (const Photon &ph : mp.at(current)) {
            if (vec3::dist(ph.pos, p) <= radius) {
              // add to the current estimate
              L += fnc(ph.wo) * ph.flux;
            }
          }
        }
      }
    }
  }
  return L;
}

spectrum
PhotonIntegrator::estimate_indirect(const SurfaceInteraction &si) const {
  const vec3 &p = si.p;
  int x = std::floor(p.x);
  int y = std::floor(p.y);
  int z = std::floor(p.z);
  auto BSDF = si.pr->get_material()->get_scatter(si);
  auto calc = [&](const vec3 &wi) { return BSDF->f(wi, si.wo, BSDF_ALL); };

  spectrum L = estimate(cell{x, y, z}, spm, calc, si.p);

  // can optimize this but whatever
  return L * INV_PI / radius / radius * inv_photons;
}

spectrum
PhotonIntegrator::estimate_indirect(const MediumInteraction &mi) const {

  const vec3 &p = mi.p;
  int x = std::floor(p.x);
  int y = std::floor(p.y);
  int z = std::floor(p.z);
  auto phase = mi.ph;
  auto calc = [&](const vec3 &wi) { return phase->p(wi, mi.wo); };

  spectrum L = estimate(cell{x, y, z}, vpm, calc, mi.p);

  // can optimize this but whatever
  // get avg illuminance divided by the kernel volume 4/3 pi r^3
  return L * INV_PI / radius / radius / radius * 3 * 0.25 * inv_photons;
}

spectrum PhotonIntegrator::Li(const ray &ra, const scene &s, RNG &rng,
                              int depth) const {
  // get the lighting - also calculates direct lighting
  ray r = ra;
  spectrum tp{1.0};
  spectrum L{0.0};

  for (int i = 0; i < m_depth; i++) {
    // bounce until we hit a diffuse surface (or scattered)
    auto y = s.intersect(r, 0);
    SurfaceInteraction &isect = *y;
    if (!y)
      isect.t = D_INFINITY;

    const medium *med = r.m;
    r.maxt = isect.t;
    MediumInteraction mi;
    if (med != nullptr) {
      tp *= med->sample(r, rng, mi);
    } else {
      if (!y)
        break;
    }

    if (mi.ph != nullptr) {
      L += tp * sample_light(mi, s, rng);

      L += estimate_indirect(mi);
      break;
    }
    // this we wanna interact with the surface instead!

    auto mat = isect.pr->get_material();
    if (mat == nullptr) {
      i--;
      r.o = isect.p;
      continue;
    }
    const AreaLight *alight = isect.pr->get_area_light();
    // not sure why we skip...
    if (alight != nullptr) {
      return L + tp * alight->Le(r);
    }

    auto BSDF = mat->get_scatter(isect);
    bxdf_t flags = BSDF->get_flags();
    if ((flags & BSDF_DIFFUSE) != 0) {
      // diffuse, use a surface estimate
      L += estimate_indirect(isect);
      return L;
    }

    // bounce
    vec3 wo = r.d, wi;
    double pdf;
    bxdf_t sampled = BSDF_NONE;
    spectrum f = BSDF->sample_f(wo, wi, isect.n, rng, pdf, BSDF_ALL, sampled);
    if (isect.mi.transition())
      r.m = isect.get_medium(wi);

    if (f.isBlack() || pdf == 0.0)
      break;
    tp *= f * std::abs(vec3::dot(isect.n, wi)) / pdf;

    r.o = isect.p;
    r.d = wi;

    r.maxt = D_INFINITY;

    if (i > 3) {
      double q = std::max(0.05, 1 - tp.magnitude() * 0.3333);
      if (rng.rfloat() < q)
        break;
      tp /= (1 - q);
    }
  }

  return L;
}

} // namespace miao
