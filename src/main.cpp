#include "miao/cameras/camera.hpp"
#include "miao/cameras/film.hpp"
#include "miao/core/material.hpp"
#include "miao/core/primitive.hpp"
#include "miao/core/scene.hpp"
#include "miao/core/shape.hpp"
#include "miao/lights/light.hpp"
#include "miao/renderers/progressive.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;
using namespace miao;

int to_8bit(double d) { return miao::max(miao::min(1.0, d), 0.0) * 255; }

void tmp(film::pix &p, int s) {
  p.r = std::sqrt(p.r / s);
  p.g = std::sqrt(p.g / s);
  p.b = std::sqrt(p.b / s);
}

int main() {
  Transformation i{};
  spectrum light{1, 1, 1};
  AreaLight alight{light};
  shared_ptr<bsdf> bs = std::make_shared<bsdf>();
  GeoPrimitive jaja{make_shared<sphere>(&i, &i, false, 2, 0, 0, 0, 0, 0), bs,
                    make_shared<AreaLight>(light)};

  vector<GeoPrimitive> x;
  x.push_back(jaja);
  // std::vector<>
  dumb_aggregate world{x};

  int width = 100;
  int height = 200;
  film f{width, height};
  scene s{{}, std::make_shared<dumb_aggregate>(world)};
  TempCamera cam{f, {0, 0, -5}, {0, 1, 0}, {0, 0, 1}, 1, 0, 90};
  ProgressiveRenderer renderer(s, cam, 10);

  auto callback = [&]() {
    freopen("frank.ppm", "w", stdout);
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
