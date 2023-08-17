#pragma once
#ifndef PPM_HPP
#define PPM_HPP

#include "miao/integrators/photon.hpp"

namespace miao {
class PPMIntegrator : public PhotonIntegrator {
public:
  PPMIntegrator(camera *cam_, int samples_ = 32, double r = 0.015,
                int np = 500000, double a_v = 0.65, double a_s = 0.7)

      : PhotonIntegrator(cam_, samples_, r, np), alpha_v(a_v), alpha_s(a_s) {}
  virtual void preprocess(const scene &s) override {
    if (iters++) {
      PhotonIntegrator::v_radius *= std::cbrt((iters + alpha_v) / (iters + 1));
      PhotonIntegrator::s_radius *= std::sqrt((iters + alpha_s) / (iters + 1));
    }
    std::cerr << "vradius is " << v_radius << " sradius is " << s_radius
              << "\n";
    PhotonIntegrator::preprocess(s);
  }

private:
  double alpha_v, alpha_s;
  int iters = 0;
};
} // namespace miao

#endif // PPM_HPP
