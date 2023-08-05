#pragma once
#include "interaction.hpp"
#ifndef MATERIAL_HPP
#define MATERIAL_HPP

// materials, bxdfs, textures.

#include "rng.hpp"
#include "spectrum.hpp"
#include "vec3.hpp"

#include <optional>

namespace miao {
namespace BXDF {
// fresnel reflectance
double FrDielectric(double costi, double etaI, double etaT);
bool refract(const vec3 &wi, const vec3 &n, double eta, vec3 &jaja);
// the schilck ap
spectrum FrConductor(double cosThetaI, const spectrum &etai,
                     const spectrum &etat, const spectrum &k);
} // namespace BXDF

enum BxDFType {
  BSDF_REFLECTION = 1 << 0,
  BSDF_TRANSMISSION = 1 << 1,
  BSDF_DIFFUSE = 1 << 2,
  BSDF_GLOSSY = 1 << 3,
  BSDF_SPECULAR = 1 << 4,
  BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION |
             BSDF_TRANSMISSION,
};

struct bxdf {
  virtual ~bxdf() {}
  bxdf(BxDFType t) : type(t) {}
  bool match(BxDFType t) const { return (t ^ type) == 0; }
  virtual spectrum f(const vec3 &wi, const vec3 &wo) const = 0;
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, RNG &rng, double &pdf,
                            BxDFType *type = nullptr) const = 0;
  virtual double pdf(const vec3 &wi, const vec3 &wo) const;

  virtual spectrum rho(const vec3 &wo, int nsamps, RNG &r) const;
  const BxDFType type;
};

class ScaledBxDF : public bxdf {
public:
  ScaledBxDF(bxdf *bxdf_, const spectrum &s)
      : bxdf(bxdf_->type), b(bxdf_), s(s) {}

  virtual double pdf(const vec3 &wi, const vec3 &wo) const {
    return b->pdf(wi, wo);
  }
  virtual spectrum rho(const vec3 &wo, int nsamps, RNG &r) const {
    return s * b->rho(wo, nsamps, r);
  }
  virtual spectrum f(const vec3 &wi, const vec3 &wo) const {
    return s * b->f(wi, wo);
  }
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, RNG &rng, double &pdf,
                            BxDFType *type = nullptr) const {
    return s * b->sample_f(wo, wi, rng, pdf, type);
  }

private:
  bxdf *b;
  spectrum s;
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

class specular : public bxdf {
public:
  specular(const spectrum &r, fresnel *f)
      : bxdf(BxDFType(BSDF_SPECULAR | BSDF_REFLECTION)), r(r), fr(f) {}

  virtual spectrum f(const vec3 &wi, const vec3 &wo) const {
    return spectrum{};
  }
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, RNG &rng, double &pdf,
                            BxDFType *type) const {

    wi = vec3{-wo.x, -wo.y, wo.z};
    pdf = 1.0;
    return fr->evaluate(wi.z) * r / std::abs(wi.z);
  }

  virtual double pdf(const vec3 &wi, const vec3 &wo) const { return 0; }

private:
  const spectrum &r;
  const fresnel *fr;
};

class transmission : public bxdf {
public:
  transmission(const spectrum &t, double etaA, double etaB, int mode)
      : bxdf(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)), T(t), etaA(etaA),
        etaB(etaB), fr(etaA, etaB) {}

  virtual spectrum f(const vec3 &, const vec3 &) const { return spectrum{}; }
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, RNG &rng, double &pdf,
                            BxDFType *type) const {
    bool entering = wo.z > 0;
    double etaI = entering ? etaA : etaB;
    double etaT = entering ? etaB : etaA;

    return spectrum{};
  }

private:
  const spectrum T;
  const double etaA, etaB;
  const frdielec fr;
};

class lambertian : public bxdf {
public:
  lambertian(const spectrum &s)
      : bxdf(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), s(s) {}

  virtual spectrum f(const vec3 &wi, const vec3 &wo) const override;
  virtual spectrum sample_f(const vec3 &wo, vec3 &wi, RNG &rng, double &pdf,
                            BxDFType *type = nullptr) const override;
  virtual double pdf(const vec3 &wi, const vec3 &wo) const override;

  virtual spectrum rho(const vec3 &wo, int nsamps, RNG &r) const override;

private:
  spectrum s;
};

class bsdf {
public:
  static constexpr int maxbxdfs = 8;
  bsdf(const SurfaceInteraction &si, double eta = 1)
      : eta(eta), si(si), x(si.dpdu.unit()), y(vec3::cross(si.n, x)) {}
  void add(bxdf *b) {
    // ASSERT(nbxdfs < maxbxdfs, "too many bxdfs uh oh");
    bxdfs[nbxdfs++] = b;
  }

  // orthonormal transformation
  int num_components(BxDFType flags = BSDF_ALL) const;
  vec3 wtl(const vec3 &v) const {
    return vec3{vec3::dot(v, x), vec3::dot(v, y), vec3::dot(v, si.n)};
  }

  // transposed transformation (unitary! so it's actually the inverse)
  vec3 ltw(const vec3 &v) const {
    return vec3{v.x * x.x + v.y * y.x + v.z * si.n.x,
                v.x * x.y + v.y * y.y + v.z * si.n.y,
                v.x * x.z + v.y * y.z + v.z * si.n.z};
  }
  spectrum f(const vec3 &wo_world, const vec3 &wi_world,
             BxDFType flags = BSDF_ALL) const;
  spectrum rho(int nsamples, RNG &rng, BxDFType flags = BSDF_ALL) const;
  spectrum rho(const vec3 &wo, int nsamples, RNG &rng,
               BxDFType flags = BSDF_ALL) const;
  spectrum sample_f(const vec3 &wo, vec3 &wi, RNG &rng, double &pdf,
                    BxDFType type = BSDF_ALL,
                    BxDFType *sampled = nullptr) const;
  double pdf(const vec3 &wo, const vec3 &wi, BxDFType flags = BSDF_ALL) const;

private:
  vec3 x, y;
  const SurfaceInteraction &si;
  double eta;
  int nbxdfs = 0;
  bxdf *bxdfs[maxbxdfs];
};

class material {
public:
  virtual void sf(SurfaceInteraction &si, int mode, bool multi_lobe) const = 0;
  virtual ~material();
};

class matte : public material {};

} // namespace miao
#endif
