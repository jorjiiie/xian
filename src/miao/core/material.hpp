#pragma once
#ifndef MATERIAL_HPP
#define MATERIAL_HPP

// materials, bxdfs, textures.

#include "rng.hpp"
#include "spectrum.hpp"
#include "vec3.hpp"

namespace BXDF {
// fresnel reflectance
double FrDielectric(double costi, double etaI, double etaT) {
  costi = clamp(costi, -1.0, 1.0);
  bool entering = costi > 0.0;
  if (!entering) {
    std::swap(etaI, etaT);
    costi = -costi;
  }
  double sini = std::sqrt(1 - costi * costi);
  double sint = etaI / etaT * sini;
  if (sint > 1.0)
    return 1;
  double cost = std::sqrt(1.0 - sint * sint);

  double rpar =
      ((etaT * costi) - (etaI * cost)) / ((etaT * costi) + (etaI * cost));
  double rperp =
      ((etaI * costi) - (etaT * cost)) / ((etaI * costi) + (etaT * cost));
  return (rpar * rpar + rperp * rperp) / 2;
}
spectrum FrConductor(double cosThetaI, const spectrum &etai,
                     const spectrum &etat, const spectrum &k) {
  cosThetaI = clamp(cosThetaI, -1.0, 1.0);
  spectrum eta = etat / etai;
  spectrum etak = k / etai;

  double cosThetaI2 = cosThetaI * cosThetaI;
  double sinThetaI2 = 1. - cosThetaI2;
  spectrum eta2 = eta * eta;
  spectrum etak2 = etak * etak;

  spectrum t0 = eta2 - etak2 - sinThetaI2;
  spectrum a2plusb2 = (t0 * t0 + eta2 * etak2 * 4).sqrt();
  spectrum t1 = a2plusb2 + cosThetaI2;
  spectrum a = ((a2plusb2 + t0) * 0.5).sqrt();
  spectrum t2 = a * 2.0 * cosThetaI;
  spectrum Rs = (t1 - t2) / (t1 + t2);

  spectrum t3 = a2plusb2 * cosThetaI2 + sinThetaI2 * sinThetaI2;
  spectrum t4 = t2 * sinThetaI2;
  spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

  return (Rp + Rs) * 0.5;
}
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

private:
  const spectrum T;
  const double etaA, etaB;
  const frdielec fr;
};

#endif
