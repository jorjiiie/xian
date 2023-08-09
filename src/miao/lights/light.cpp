#include "miao/lights/light.hpp"

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
} // namespace miao
