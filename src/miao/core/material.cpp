#include "miao/core/material.hpp"
#include "miao/core/interaction.hpp"

#include "miao/core/debug.hpp"

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
// https://graphics.pixar.com/library/OrthonormalB/paper.pdf (but it's the
// frisvad one)
void onb(const vec3 &i, vec3 &j, vec3 &k) {
  if (i.z < -0.9999) {
    j = vec3{0, -1, 0};
    k = vec3{-1, 0, 0};
    return;
  }
  const double a = 1.0 / (1.0 + i.z);
  const double b = -i.x * i.y * a;
  j = vec3{1.0 - i.x * i.x * a, b, -i.x};
  k = vec3{b, 1.0 - i.y * i.y * a, -i.y};
}
vec3 changebasis(
    const vec3 &n,
    const vec3 &w) { // changes w from w in n = (0,0,1) space to world space
  vec3 j, k;
  const vec3 &i = n;
  onb(i, j, k);

  return vec3{
      w.x * i.x + w.y * j.x + w.z * k.x,
      w.x * i.y + w.y * j.y + w.z * k.y,
      w.x * i.z + w.y * j.z + w.z * k.z,
  };
}
vec3 cosine_unit(RNG &rng) {
  double t = rng.rfloat() * 2 * PI;
  double v = std::sqrt(rng.rfloat());

  double x = std::cos(t) * v;
  double z = std::sin(t) * v;
  double y = std::sqrt(max(0.0, 1.0 - x * x - z * z));

  return vec3{x, y, z};
}
} // namespace BXDF

spectrum lambertbsdf::f(const vec3 &wi, const vec3 &wo, const vec3 &n) const {
  return s;
}
spectrum lambertbsdf::sample_f(const vec3 &wo, vec3 &wi, const vec3 &n,
                               RNG &rng, double &pdf) const {

  vec3 w = BXDF::cosine_unit(rng);
  vec3 j, k;
  const vec3 &i = n;
  // i j k defines the transformation
  BXDF::onb(i, j, k);
  wi = vec3{
      w.x * i.x + w.y * j.x + w.z * k.x,
      w.x * i.y + w.y * j.y + w.z * k.y,
      w.x * i.z + w.y * j.z + w.z * k.z,
  };

  DEBUG(i.ts(), j.ts(), k.ts(), w.ts(), wi.ts());
  pdf = this->pdf(wi, wo, n);
  return f(wi, wo, n);
}

spectrum bsdf::f(const vec3 &wi, const vec3 &wo, bool refl) const {
  spectrum f{};
  for (auto &x : bxdfs) {
    f += x->f(wi, wo, this->n);
  }
  return f;
}

spectrum bsdf::sample_f(const vec3 &wo, vec3 &wi, const vec3 &n, RNG &rng,
                        double &pdf) const {
  return bxdfs[0]->sample_f(wo, wi, n, rng, pdf);
}

std::shared_ptr<bsdf> lambert::get_scatter(SurfaceInteraction &si) const {
  auto y = std::make_shared<bsdf>(si, 1);
  y->add(std::make_shared<lambertbsdf>(s));
  return y;
}

} // namespace miao
