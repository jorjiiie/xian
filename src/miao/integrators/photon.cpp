#include "miao/integrators/photon.hpp"

#include "miao/cameras/camera.hpp"
#include "miao/cameras/film.hpp"

#include "miao/core/distribution.hpp"
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

int64_t good_photons = 0;
int64_t tot_photons = 0;

spectrum PhotonIntegrator::sample_lights(const scene &s, ray &r,
                                         discrete_1d &distr, RNG &rng) const {
  int sampled_light = clamp(distr.sample(rng), 0, (int)s.lights.size() - 1);

  auto li = s.lights[sampled_light];

  double pdf;
  auto x = li->sample(r, rng, pdf);
  DEBUG("THROUGHPUT?? ", x.ts(), " ", pdf);

  return x / pdf;
}

void PhotonIntegrator::preprocess(const scene &s) {
  // builds the photon map.

  vpm.clear();
  spm.clear();
  DEBUG("CURRENTLY SHOOTING PHOTONS");
  int num_shooters = 1;
#ifdef _OPENMP
  num_shooters = omp_get_max_threads();
  omp_set_num_threads(num_shooters);
#endif

  std::vector<double> light_powers;
  for (const auto &ptr : s.lights) {
    light_powers.push_back(ptr->power().magnitude());
  }

  discrete_1d li_distr(light_powers);
  std::vector<pcg32> rngs(num_shooters);
  for (int i = 0; i < num_shooters; i++) {
    rngs[i].seed(rand() * rand());
  }

  int vol_photons = 0, s_photons = 0;
  auto shoot = [&]() {
    int idx = 0;
    int to_shoot = num_photons / num_shooters;
#ifdef _OPENMP
    idx = omp_get_thread_num();

    if (to_shoot + 1 == num_shooters) {
      // must do the remainder as well!
      to_shoot += num_photons % num_shooters;
    }
#endif
    RNG &rng = rngs[idx];
    for (int shot = 0; shot < to_shoot; shot++) {
      // sample the scene lights;
      ray r;

      spectrum tp = sample_lights(s, r, li_distr, rng);
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
            cell c = get_cell(mi.p);
#pragma omp critical
            {
              ++vol_photons;
              // this needs to account for the fraction that gets boosted?
              // sigma_s
              vpm[c].push_back(Photon{mi.p, mi.wo, tp});
              /* DEBUG("DEPOSITING A VOLUME PHOTON WITH TP", tp.ts(), " at cell
               * ", */
              /*       c.i, ' ', c.j, ' ', c.k, " with point ", mi.p.ts()); */
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
            cell c = get_cell(isect.p);
#pragma omp critical
            {
              spm[c].push_back({Photon{isect.p, isect.wo, tp}});
              ++s_photons;
              if (isect.p[1] < 6)
                DEBUG("DEPOSITING A SURFACE PHOTON WITH TP", tp.ts(),
                      " at cell ", c.i, ' ', c.j, ' ', c.k, " with point ",
                      isect.p.ts(), r.o.ts(), r.d.ts(), mat);
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
          r.o = isect.p + wi * vec3::eps;
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

  DEBUG("DONE SHOOTING PHOTONS ", vol_photons, " ", s_photons);
  std::cerr << "done shooting photons, statistics is " << good_photons << " "
            << tot_photons << " for " << good_photons * 1.0 / tot_photons << " "
            << vpm.size() << " " << spm.size() << "\n";
  /* for (const auto &z : spm) { */
  /*   const cell &c = z.first; */
  /*   std::cerr << "cell " << c.i << " " << c.j << " " << c.k << "\n"; */
  /* } */
  // oof
}

spectrum PhotonIntegrator::estimate(
    const cell &c,
    const std::unordered_map<cell, std::vector<Photon>, hash> &mp,
    const std::function<spectrum(const vec3 &)> &fnc, const vec3 &p) const {
  spectrum L{};

  const int &x = c.i;
  const int &y = c.j;
  const int &z = c.k;

  int n_phots = 0;
  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      for (int dz = -1; dz <= 1; dz++) {
        cell current{x + dx, y + dy, z + dz};
        if (mp.find(current) != mp.end()) {
          // go through photons
          for (const Photon &ph : mp.at(current)) {

            tot_photons++;

            if ((ph.pos - p).msq() <= radius * radius) {
              // add to the current estimate
              L += fnc(ph.wo) * ph.flux;
              n_phots++;
              good_photons++;
            }
          }
        }
      }
    }
  }
  DEBUG(n_phots, " ", L.ts());
  return L;
}

spectrum
PhotonIntegrator::estimate_indirect(const SurfaceInteraction &si) const {
  const vec3 &p = si.p;
  cell c = get_cell(p);
  auto BSDF = si.pr->get_material()->get_scatter(si);
  auto calc = [&](const vec3 &wi) { return BSDF->f(wi, si.wo, BSDF_ALL); };

  spectrum L = estimate(c, spm, calc, si.p);

  // can optimize this but whatever
  return L * INV_PI / radius / radius * inv_photons;
}

spectrum
PhotonIntegrator::estimate_indirect(const MediumInteraction &mi) const {

  const vec3 &p = mi.p;
  cell c = get_cell(p);
  auto phase = mi.ph;
  auto calc = [&](const vec3 &wi) { return phase->p(wi, mi.wo); };

  spectrum L = estimate(c, vpm, calc, mi.p);

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

      spectrum spec = estimate_indirect(mi);
      // DEBUG(spec.ts());
      L += tp * spec;
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
      spectrum spec = estimate_indirect(isect);
      spectrum li = sample_light(isect, s, rng);
      DEBUG(spec.ts(), li.ts(), " ", i, " ", isect.p.ts(), tp.ts());
      L += tp * spec;
      L += tp * li;
      DEBUG("WTF ", L.ts());
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
