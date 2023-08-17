#include "miao/cameras/camera.hpp"
#include "miao/cameras/film.hpp"
#include "miao/core/distribution.hpp"
#include "miao/core/interaction.hpp"
#include "miao/core/material.hpp"
#include "miao/core/primitive.hpp"
#include "miao/core/scene.hpp"
#include "miao/core/shape.hpp"
#include "miao/core/transform.hpp"
#include "miao/core/utils.hpp"
#include "miao/ds/bvh.hpp"
#include "miao/integrators/photon.hpp"
#include "miao/integrators/ppm.hpp"
#include "miao/integrators/volumeintegrator.hpp"
#include "miao/lights/light.hpp"
#include "miao/renderers/progressive.hpp"
#include "miao/volumes/medium.hpp"

#include "miao/core/debug.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;
using namespace miao;

int64_t cnter_::bbox_tests = 0;
int64_t cnter_::prim_tests = 0;
int64_t cnter_::rays_cast = 0;
int64_t cnter_::good_tests = 0;
int to_8bit(double d) { return miao::max(miao::min(1.0, d), 0.0) * 255; }

void tmp(film::pix &p, int s) {

  p.r = std::sqrt(p.r / s);
  p.g = std::sqrt(p.g / s);
  p.b = std::sqrt(p.b / s);
}

int main() {
  scene s;
  s.load_from_obj("models/box.obj");

  Transformation z = Transformation::translate(vec3{0.4, .855, 0.2}) *
                     Transformation::scale(0.2, 0.2, 0.2) *
                     Transformation::rotateY(180);
  s.load_from_obj("models/tyra.obj", z);

  Transformation szz = Transformation::translate(vec3{-0.5, 0.35, 0.25}) *
                       Transformation::scale(0.3, 0.3, 0.3) *
                       Transformation::rotateY(20) *
                       Transformation::rotateX(-30);
  s.load_from_obj("models/suzanne.obj", szz);

  Transformation fish = Transformation::translate(vec3{-.3, 1.3, -0.2}) *
                        Transformation::scale(0.1, 0.1, 0.1) *
                        Transformation::rotateY(30);
  s.load_from_obj("models/fish.obj", fish);

  s.build();

  // spectrum li{20 / 10, 10 / 10, 6 / 10};
  spectrum li{4, 3, 3};

  // Medium med{spectrum{}}
  homogeneous MEDIUM{spectrum{0.1, 0.05, 0.005}, spectrum{0.05, 0.08, 0.15}, 0};
  // const medium *medptr = nullptr;
  const medium *medptr = &MEDIUM;
  vec3 dD{0, 2, .1};
  Transformation tt = Transformation::translate(dD);
  Transformation invtt = Transformation::translate(-dD);
  std::shared_ptr<shape> lc =
      std::make_shared<sphere>(&tt, &invtt, false, .02, -2, 2, 0, 0, 0);

  std::shared_ptr<AreaLight> alight =
      std::make_shared<AreaLight>(li * 1.5 * 3, lc);
  alight->m = medptr;

  s.add_prim(std::make_shared<GeoPrimitive>(
      lc, std::make_shared<lambert>(spectrum{1.0}), alight));
  s.lights.push_back(alight);

  int width = 1000;
  int height = 1000;
  film f{width, height};

  TempCamera cam{f, {0, 1, 2}, {0, 1, 0}, {0, 1, 0}, 1, 0, 90};
  cam.med = medptr;
  ProgressiveRenderer<PPMIntegrator> renderer(s, cam, 1500, 2);

  auto callback = [&](int x) {
    freopen(("ppm/tyra" + to_string(x) + ".ppm").c_str(), "w", stdout);

    std::cerr << "bbox tests: " << cnter_::bbox_tests << "\n"
              << " primitive tests: " << cnter_::prim_tests
              << "\nrays cast:" << cnter_::rays_cast
              << "\n prims hit: " << cnter_::good_tests << "\n";
    cout << "P3 " << width << " " << height << "\n255\n";
    for (int i = height - 1; i >= 0; --i) {
      for (int j = 0; j < width; j++) {
        auto y = f.pixel(j, i);
        tmp(y.first, y.second);
        int r = to_8bit(y.first.r);
        int g = to_8bit(y.first.g);
        int b = to_8bit(y.first.b);
        cout << r << " " << g << " " << b << "\n";
      }
    }

    freopen("RENDER_STATE.txt", "w", stdout);
    // for raw data and combining them!

    for (int i = height - 1; i >= 0; --i) {
      for (int j = 0; j < width; j++) {
        auto y = f.pixel(j, i);
        std::cout << y.first.r << " " << y.first.g << " " << y.first.b << " "
                  << y.second << "\n";
      }
    }
  };
  renderer.render(callback);
}
