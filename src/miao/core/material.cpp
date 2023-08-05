#include "material.hpp"
#include "interaction.hpp"

namespace miao {

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
bool refract(const vec3 &wi, const vec3 &n, double eta, vec3 &jaja) {
  double cosi = vec3::dot(wi, n);
  double sin2i = 1.0 - cosi * cosi;
  double sin2t = eta * eta * sin2i;
  if (sin2t >= 1.0)
    return false;
  double cost = std::sqrt(1 - sin2t);
  jaja = eta * -wi + (eta * cosi - cost) * n;
  return true;
}

// the schilck ap
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

spectrum lambertian::f(const vec3 &wi, const vec3 &wo) const {
  return s * INV_PI;
}
double lambertian::pdf(const vec3 &wi, const vec3 &wo) const {
  return vec3::dot(wi, wo);
}
spectrum lambertian::rho(const vec3 &wo, int nsamps, RNG &r) const { return s; }

spectrum bsdf::f(const vec3 &wo_world, const vec3 &wi_world,
                 BxDFType flags) const {
  spectrum out{};
  vec3 wo = wtl(wo_world);
  vec3 wi = wtl(wi_world);
  // can't we just dot the wi wo together? lol
  bool reflect = vec3::dot(wo_world, si.n) * vec3::dot(wi_world, si.n) > 0;

  for (int i = 0; i < nbxdfs; i++) {
    if (bxdfs[i]->match(flags) &&
        ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
         (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION)))) {
      out += bxdfs[i]->f(wo, wi);
    }
  }
  return out;
}

} // namespace miao
