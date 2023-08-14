#include "miao/lights/light.hpp"

#include "miao/core/material.hpp"
#include "miao/core/scene.hpp"
#include "miao/core/shape.hpp"

namespace miao {
spectrum AreaLight::Li(const interaction &si, RNG &rng, vec3 &wi, double &pdf,
                       visibility *v) const {
  interaction ss = this->base->sample(si, rng);
  vec3 dx = (ss.p - si.p);
  wi = dx.unit();
  pdf = dx.msq() / (std::abs(vec3::dot(ss.n, -wi)) * base->area());
  *v = visibility(si, ss);

  return emit;
}
spectrum AreaLight::power() const { return emit * area * PI; }
double AreaLight::pdf_li(const interaction &si, const vec3 &wi) const {
  return this->base->pdf(si, wi);
}

spectrum AreaLight::sample(ray &r, RNG &rng, double &pdf) const {
  interaction it = this->base->sample(rng);
  // for now i guess we just sample everywhere lmfao
  // random in sphere? nah cosine weighted but two sided?
  vec3 wi = BXDF::cosine_unit(rng);
  wi = BXDF::changebasis(it.n, wi);

  // note that we only emit from one side here - make sure you don't emit to
  // other side then
  pdf = std::abs(vec3::dot(it.n, wi)) / base->area() * INV_PI;
  r = ray{it.p + wi * vec3::eps * vec3::eps, wi, 0};
  r.m = m;

  return Le(r) * std::abs(vec3::dot(it.n, wi));
}
} // namespace miao
