#pragma once
#include <iostream>
#ifndef MATERIAL_HPP
#define MATERIAL_HPP

// materials, bxdfs, textures.

#include "miao/core/interaction.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/spectrum.hpp"
#include "miao/core/vec3.hpp"

#include "miao/core/debug.hpp"

#include <memory>
#include <optional>

namespace miao {
template <typename T> class texture;
namespace BXDF {
// fresnel reflectance
double FrDielectric(double costi, double etaI, double etaT);
bool refract(const vec3 &wi, const vec3 &n, double eta, vec3 &jaja);
// the schilck ap
spectrum FrConductor(double cosThetaI, const spectrum &etai,
                     const spectrum &etat, const spectrum &k);

void onb(const vec3 &i, vec3 &j, vec3 &k);

// changes w from w in n = (0,1,0) space to world space
vec3 changebasis(const vec3 &n, const vec3 &w);
vec3 cosine_unit(RNG &rng);
} // namespace BXDF

enum bxdf_t {
  BSDF_NONE = 0,
  BSDF_REFLECTION = 1 << 0,
  BSDF_TRANSMISSION = 1 << 1,
  BSDF_DIFFUSE = 1 << 2,
  BSDF_GLOSSY = 1 << 3,
  BSDF_SPECULAR = 1 << 4,
  BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION |
             BSDF_TRANSMISSION,
};

// for btdf and brdfs
struct bxdf {
  bxdf(bxdf_t t_) : t(t_) {}
  virtual ~bxdf() {}
  virtual spectrum f(const vec3 &wi, const vec3 &wo, const vec3 &n) const = 0;
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, const vec3 &n, RNG &rng,
                            double &pdf) const {
    wi = BXDF::cosine_unit(rng);
    wi = BXDF::changebasis(n, wi);
    if (vec3::dot(wo, n) > 0) {
      wi = -wi;
    }
    pdf = this->pdf(wi, wo, n);
    return this->f(wi, wo, n);
  }
  virtual double pdf(const vec3 &wi, const vec3 &wo, const vec3 &n) const {
    if (vec3::dot(wi, n) * vec3::dot(wo, n) > 0) {
      return 0;
    }
    return std::abs(vec3::dot(wi, n)) * INV_PI;
  }
  bxdf_t t;
};
class fresnel {
public:
  // virtual ~fresnel();
  virtual spectrum evaluate(double cosi) const = 0;
};

class frcond : public fresnel {
public:
  virtual spectrum evaluate(double cosi) const {
    return BXDF::FrConductor(std::abs(cosi), etaI, etaT, k);
  }
  frcond(const spectrum &etaI, const spectrum &etaT, const spectrum &k)
      : etaI(etaI), etaT(etaT), k(k) {}

private:
  spectrum etaI, etaT, k;
};

class frdielec : public fresnel {
public:
  virtual spectrum evaluate(double cosi) const {
    return BXDF::FrDielectric(cosi, etaI, etaT);
  }
  frdielec(double etaI, double etaT) : etaI(etaI), etaT(etaT) {}

private:
  double etaI, etaT;
};

class ezfresnel : public fresnel {
public:
  virtual spectrum evaluate(double) const override { return spectrum{1.}; }
  ezfresnel() {}
};

class lambertbsdf : public bxdf {
public:
  lambertbsdf(const spectrum &s_)
      : bxdf(bxdf_t(BSDF_REFLECTION | BSDF_DIFFUSE)), s(s_) {}
  virtual spectrum f(const vec3 &wi, const vec3 &wo,
                     const vec3 &n) const override;

private:
  spectrum s;
};

class specularbsdf : public bxdf {
public:
  specularbsdf(const spectrum &s_, fresnel *f)
      : bxdf(bxdf_t(BSDF_SPECULAR | BSDF_REFLECTION)), s(s_), fr(f) {}
  virtual spectrum f(const vec3 &wi, const vec3 &wo,
                     const vec3 &n) const override;
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, const vec3 &n, RNG &rng,
                            double &pdf) const override;
  virtual double pdf(const vec3 &, const vec3 &, const vec3 &) const override {
    return 0;
  }

private:
  spectrum s;
  fresnel *fr;
};

// scaling on particle transport
class specularbtdf : public bxdf {
public:
  specularbtdf(const spectrum &s_, double etaA_, double etaB_)
      : bxdf(bxdf_t(BSDF_SPECULAR | BSDF_TRANSMISSION)), s(s_), etaA(etaA_),
        etaB(etaB_), fr(etaA_, etaB_) {}
  virtual spectrum f(const vec3 &wi, const vec3 &wo,
                     const vec3 &n) const override;
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, const vec3 &n, RNG &rng,
                            double &pdf) const override;
  virtual double pdf(const vec3 &, const vec3 &, const vec3 &) const override {
    return 0;
  }

private:
  spectrum s;
  double etaA, etaB;
  frdielec fr;
};

// handles total internal reflection
class specular : public bxdf {
public:
  specular(const spectrum &R, const spectrum &T, double etaA_, double etaB_)
      : bxdf(bxdf_t(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR)), r(R),
        t(T), fr(etaA_, etaB_), etaA(etaA_), etaB(etaB_) {}
  virtual spectrum f(const vec3 &, const vec3 &, const vec3 &) const override {
    return spectrum{};
  }
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, const vec3 &n, RNG &rng,
                            double &pdf) const override;
  double pdf(const vec3 &, const vec3 &, const vec3 &) const override {
    return 0;
  }

private:
  spectrum r, t;
  double etaA, etaB;
  frdielec fr;
};

// collection of bxdfs - will be held by the primitives
// damn a bsdf is created at each intersection point!
// for now we have single bxdf materials
class bsdf {
public:
  static constexpr int MAX = 8;

  bsdf() {}
  bsdf(const SurfaceInteraction &si_, double e = 1) : eta(e), si(si_) {}
  void add(std::shared_ptr<bxdf> b) { bxdfs[nb++] = b; }
  spectrum f(const vec3 &wi, const vec3 &wo, bxdf_t types) const;
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, const vec3 &n, RNG &rng,
                            double &pdf, bxdf_t types, bxdf_t &sampled) const;
  bxdf_t get_flags() const {
    bxdf_t flags = BSDF_NONE;
    for (int i = 0; i < nb; i++) {
      flags = bxdf_t(flags | bxdfs[i]->t);
    }
    return flags;
  }
  double pdf(const vec3 &wi, const vec3 &wo, const vec3 &n,
             bxdf_t types = BSDF_ALL) const {
    double tot = 0;
    for (int i = 0; i < nb; i++) {
      if (bxdfs[i]->t & types)
        tot += bxdfs[i]->pdf(wi, wo, n);
    }
    return tot;
  }

private:
  std::shared_ptr<bxdf> bxdfs[MAX];
  int nb = 0;
  double eta;
  SurfaceInteraction si;
};

class material {
public:
  virtual std::shared_ptr<bsdf>
  get_scatter(const SurfaceInteraction &s) const = 0;
};

class lambert : public material {
public:
  lambert(const spectrum &s_) : s(s_) {}

  virtual std::shared_ptr<bsdf>
  get_scatter(const SurfaceInteraction &s) const override;

private:
  spectrum s;
};

class metal : public material {
public:
  metal(const spectrum &s_,
        std::shared_ptr<fresnel> f = std::make_shared<ezfresnel>())
      : fr(f), s(s_) {}
  virtual std::shared_ptr<bsdf>
  get_scatter(const SurfaceInteraction &s) const override;

private:
  std::shared_ptr<fresnel> fr;
  spectrum s;
};

class glass : public material {
public:
  glass(const spectrum &s_, double etaA_, double etaB_)
      : etaA(etaA_), etaB(etaB_), s(s_) {}
  virtual std::shared_ptr<bsdf>
  get_scatter(const SurfaceInteraction &s) const override;

private:
  spectrum s;
  double etaA, etaB;
};

} // namespace miao
#endif
