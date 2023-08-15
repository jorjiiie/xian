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
  scene s{"models/box.obj"};

  int width = 1000;
  int height = 1000;
  film f{width, height};

  TempCamera cam{f, {0, 1, 2}, {0, 1, 0}, {0, 1, 0}, 1, 0, 90};
  cam.med = nullptr;
  ProgressiveRenderer<PPMIntegrator> renderer(s, cam, 15, 15);

  auto callback = [&](int x) {
    freopen(("aa" + to_string(x) + ".ppm").c_str(), "w", stdout);

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
  };
  renderer.render(callback);
}
