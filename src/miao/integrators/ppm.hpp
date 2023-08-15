#pragma once
#ifndef PPM_HPP
#define PPM_HPP

#include "miao/integrators/photon.hpp"

namespace miao {
class PPMIntegrator : public PhotonIntegrator {
public:
  PPMIntegrator(camera *cam, int samples = 32, double r = 0.05,
                int np = 2500000, double a_v = 0.4, double a_s = 0.4)

      : PhotonIntegrator(cam, samples, r, np), alpha_v(a_v), alpha_s(a_s) {}
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
