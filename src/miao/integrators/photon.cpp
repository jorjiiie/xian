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
#include <unordered_map>

namespace miao {

int64_t good_photons = 0;
int64_t tot_photons = 0;
int64_t v_avg = 0;
int64_t v_amt = 0;

spectrum PhotonIntegrator::sample_lights(const scene &s, ray &r,
                                         discrete_1d &distr, RNG &rng,
                                         spectrum tot) const {
  int sampled_light = clamp(distr.sample(rng), 0, (int)s.lights.size() - 1);

  auto li = s.lights[sampled_light];

  double pdf;
  auto x = li->sample(r, rng, pdf);

  return x / pdf * tot / li->power();
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
  spectrum total_power{};
  for (const auto &ptr : s.lights) {
    light_powers.push_back(ptr->power().magnitude());
    total_power += ptr->power();
  }

  discrete_1d li_distr(light_powers);
  std::vector<pcg32> rngs(num_shooters);
  for (int i = 0; i < num_shooters; i++) {
    rngs[i].seed(rand() * rand());
  }
  std::vector<std::unordered_map<cell, std::vector<Photon>, hash>> vmps(
      num_shooters);
  std::vector<std::unordered_map<cell, std::vector<Photon>, hash>> smps(
      num_shooters);

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

      spectrum tp = sample_lights(s, r, li_distr, rng, total_power);
      DEBUG(tp.ts(), " sick tp bro");
      // we only sample indirect illumination
      bool interacted = true;
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
          if (interacted || true) {
            cell c = get_cell_v(mi.p);
            {
              // this needs to account for the fraction that gets
              // boosted? sigma_s
              vmps[idx][c].push_back(Photon{mi.p, mi.wo, tp});
              // vpm[c].push_back(Photon{
              // mi.p, mi.wo, tp /* * static_cast<const homogeneous *>(med)->ss
              // */});
              DEBUG("DEPOSITING A VOLUME PHOTON WITH TP", tp.ts(), " at cell",
                    c.i, ' ', c.j, ' ', c.k, " with point ", mi.p.ts());
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
            cell c = get_cell_s(isect.p);
            {
              smps[idx][c].push_back(Photon{isect.p, isect.wo, tp});
              DEBUG("DEPOSITING A SURFACE PHOTON WITH TP", tp.ts(), " at cell ",
                    c.i, ' ', c.j, ' ', c.k, " with point ", isect.p.ts(),
                    r.o.ts(), r.d.ts(), mat);
            }
          }
          interacted = true;
          vec3 wo = r.d, wi;
          double pdf;
          bxdf_t sampled = BSDF_NONE;
          spectrum f =
              b->sample_f(wo, wi, isect.n, rng, pdf, BSDF_ALL, sampled);

          // hack for now
          // if (isect.mi.transition())
          // r.m = isect.get_medium(wi);

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

  int huh = 0;
  for (int i = 0; i < num_shooters; i++) {
    for (const auto &[cell, photons] : vmps[i]) {
      huh += photons.size();
      vpm[cell].insert(vpm[cell].end(), photons.begin(), photons.end());
    }
    for (const auto &[cell, photons] : smps[i]) {
      spm[cell].insert(spm[cell].end(), photons.begin(), photons.end());
    }
  }

  std::cerr << "huh photons bro " << huh << "\n";

  std::cerr << "DONE SHOOTING PHOTONS " << vol_photons << " " << s_photons
            << "\n";
  std::cerr << "done shooting photons, statistics is " << good_photons << " "
            << tot_photons << " for " << good_photons * 1.0 / tot_photons << " "
            << vpm.size() << " " << spm.size() << " " << v_amt << " " << v_avg
            << " " << v_avg * 1.0 / v_amt << "\n";
  /* for (const auto &z : spm) { */
  /*   const cell &c = z.first; */
  /*   std::cerr << "cell " << c.i << " " << c.j << " " << c.k << "\n"; */
  /* } */
  // oof
}

spectrum PhotonIntegrator::estimate(
    const cell &c,
    const std::unordered_map<cell, std::vector<Photon>, hash> &mp,
    const std::function<spectrum(const vec3 &)> &fnc, const vec3 &p,
    double radius) const {
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
  if (mp.size() == vpm.size()) {
    v_avg += n_phots;
    v_amt++;
    DEBUG(n_phots, " volume estimate! ");
  } else {
    DEBUG("surface estimate!");
  }
  DEBUG(n_phots, " ", L.ts());
  return L;
}

spectrum
PhotonIntegrator::estimate_indirect(const SurfaceInteraction &si) const {
  const vec3 &p = si.p;
  cell c = get_cell_s(p);
  auto BSDF = si.pr->get_material()->get_scatter(si);
  auto calc = [&](const vec3 &wi) { return BSDF->f(wi, si.wo, BSDF_ALL); };

  spectrum L = estimate(c, spm, calc, si.p, s_radius);

  // can optimize this but whatever
  return L * INV_PI / s_radius / s_radius * inv_photons;
}

spectrum
PhotonIntegrator::estimate_indirect(const MediumInteraction &mi) const {
  // std::cerr << "what\n";
  const vec3 &p = mi.p;
  cell c = get_cell_v(p);
  auto phase = mi.ph;
  auto calc = [&](const vec3 &wi) { return phase->p(wi, mi.wo); };

  spectrum L = estimate(c, vpm, calc, mi.p, v_radius);

  // can optimize this but whatever
  // get avg illuminance divided by the kernel volume 4/3 pi r^3
  return L * INV_PI / v_radius / v_radius / v_radius * 3 * 0.25 * inv_photons;
}

spectrum PhotonIntegrator::estimate_indirect_v(const ray &r,
                                               const PhaseFunction &ph) const {
  cell c = get_cell_v(r.o);
  auto calc = [&](const vec3 &wi) { return ph.p(wi, r.d); };
  spectrum L = estimate(c, vpm, calc, r.o, v_radius);

  return L * INV_PI / v_radius / v_radius / v_radius * 3 * 0.25 * inv_photons;
}

spectrum PhotonIntegrator::estimate_beam(const ray &ra, double t,
                                         const medium *med, spectrum &tp,
                                         RNG &rng) const {
  if (med == nullptr)
    return spectrum{};
  ray r = ra;
  auto phase_func = med->get_ph();
  if (t < default_dist) {
    // uhh we just take the estimate here and just nothing else?
    tp = med->tr(r, r.maxt, rng);
    return estimate_indirect_v(r, *phase_func);
  }
  // recursively split this up.
  // calculate this stuff
  spectrum Li{};

  t = min(t, max_dist);
  for (double x = 0; x < t; x += default_dist) {
    Li += estimate_indirect_v(r, *med->get_ph()) *
          min(default_dist, t - default_dist) / med->sig_t(r.o);
    tp *= (-med->sig_t(r.o) * default_dist).exp();
    r.o = r.o + r.d * default_dist;
  }
  return Li;
}

spectrum PhotonIntegrator::Li(const ray &ra, const scene &s, RNG &rng,
                              int) const {
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

    spectrum tp_m{1.0};
    spectrum Li_medium = estimate_beam(r, r.maxt, med, tp_m, rng);
    L += tp * Li_medium;
    tp *= tp_m;
    if (!y)
      return L;

    // continue the light
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
      // spectrum li = sample_light(isect, s, rng);
      //   DEBUG(spec.ts(), li.ts(), " ", i, " ", isect.p.ts(), tp.ts());
      L += tp * spec;
      // L += tp * li;
      return L;
    }

    // bounce
    vec3 wo = r.d, wi;
    double pdf;
    bxdf_t sampled = BSDF_NONE;
    spectrum f = BSDF->sample_f(wo, wi, isect.n, rng, pdf, BSDF_ALL, sampled);

    // hack for now
    // if (isect.mi.transition())
    // r.m = isect.get_medium(wi);

    if (f.isBlack() || pdf == 0.0)
      break;
    tp *= f * std::abs(vec3::dot(isect.n, wi)) / pdf;

    r.o = isect.p + wi * vec3::eps;
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
