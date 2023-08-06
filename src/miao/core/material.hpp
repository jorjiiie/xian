#pragma once
#ifndef MATERIAL_HPP
#define MATERIAL_HPP

// materials, bxdfs, textures.

#include "interaction.hpp"
#include "rng.hpp"
#include "spectrum.hpp"
#include "vec3.hpp"

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
vec3 changebasis(const vec3 &n, const vec3 &v);
vec3 cosine_unit(RNG &rng);
} // namespace BXDF

// for btdf and brdfs
struct bxdf {
  virtual ~bxdf() {}
  bxdf(bool specular = false) : is_specular(specular) {}
  virtual spectrum f(const vec3 &wi, const vec3 &wo, const vec3 &n) const = 0;
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, const vec3 &n, RNG &rng,
                            double &pdf) const {
    wi = BXDF::cosine_unit(rng);
    if (vec3::dot(wo, n) > 0) {
      wi = -wi;
    }
    wi = BXDF::changebasis(n, wi);
    pdf = this->pdf(wi, wo, n);
    return this->f(wi, wo, n);
  }
  virtual double pdf(const vec3 &wi, const vec3 &wo, const vec3 &n) const {
    if (vec3::dot(wi, wo) > 0) {
      return 0;
    }
    return std::abs(vec3::dot(wi, n)) * INV_PI;
  }

  bool is_specular = false;
};
class fresnel {
public:
  virtual ~fresnel();
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

class lambertbsdf : public bxdf {
public:
  lambertbsdf(const spectrum &s) : s(s) {}
  virtual spectrum f(const vec3 &wi, const vec3 &wo,
                     const vec3 &n) const override;
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, const vec3 &n, RNG &rng,
                            double &pdf) const override;

private:
  spectrum s;
};

class specularbsdf : public bxdf {
public:
  specularbsdf(const spectrum &s, fresnel *f) : s(s), fr(f) {}
  virtual spectrum f(const vec3 &wi, const vec3 &wo,
                     const vec3 &n) const override;
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, const vec3 &n, RNG &rng,
                            double &pdf) const override;

private:
  spectrum s;
  fresnel *fr;
};

// collection of bxdfs - will be held by the primitives
class bsdf {
public:
  static constexpr int MAX = 8;
  bsdf(const SurfaceInteraction &si, double e) : si(si), eta(e), n(si.n) {}
  void add(std::shared_ptr<bxdf> b) { bxdfs[nb++] = b; }
  spectrum f(const vec3 &wi, const vec3 &wo, bool refl) const;
  spectrum sample_f(const vec3 &wo, vec3 &wi, const vec3 &n, RNG &rng,
                    double &pdf) const;

private:
  std::shared_ptr<bxdf> bxdfs[MAX];
  int nb = 0;
  double eta;
  vec3 n;
  SurfaceInteraction si;
};

} // namespace miao
#endif
