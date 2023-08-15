#include "miao/core/interaction.hpp"
#include "miao/core/material.hpp"
#include "miao/core/primitive.hpp"
#include "miao/core/ray.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/shape.hpp"
#include "miao/core/transform.hpp"
#include <iostream>
#include <memory>

using namespace miao;
using namespace std;

int main() {
  Transformation id;
  sphere s{&id, &id, false, 1, 0, 0, 0, 0, 0};
  ray r{vec3{-2, 0, 0}, vec3{2.0 / sqrt(5), -1.0 / sqrt(5), 0}, 0};

  shared_ptr<material> x =
      make_shared<glass>(spectrum{1.0, 1.0, 1.0}, 1.0, 1.45);
  double t;
  SurfaceInteraction isect;
  s.intersect(r, t, isect);
  auto y = x->get_scatter(isect);
  vec3 wi;
  bxdf_t sampled;
  double pdf;
  pcg32 rng;
  y->sample_f(r.d, wi, isect.n, rng, pdf, BSDF_ALL, sampled);
  cout << t << (r.o + t * r.d).ts() << " " << isect.p.ts()

       << "\n";
  cout << wi.ts() << "\n";
}
