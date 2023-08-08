#include "miao/cameras/camera.hpp"
#include "miao/cameras/film.hpp"
#include "miao/core/material.hpp"
#include "miao/core/primitive.hpp"
#include "miao/core/scene.hpp"
#include "miao/core/shape.hpp"
#include "miao/core/transform.hpp"
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

  Transformation id{};
  spectrum light{1, 1, 1};
  AreaLight alight{light};

  shared_ptr<material> bs = make_shared<lambert>(spectrum{.5, .5, .5});
  vec3 dD{1, 1, 0};
  Transformation tt = Transformation::translate(dD);
  Transformation invtt = Transformation::translate(-dD);
  Transformation vertshift = Transformation::translate({0, 5, 0});
  Transformation ov = Transformation::translate({0, -5, 0});

  Transformation move = Transformation::translate({-1, -2, 1});
  Transformation back = Transformation::translate({1, 2, -1});

  GeoPrimitive jaja{make_shared<sphere>(&tt, &invtt, false, 2, 0, 0, 0, 0, 0),
                    bs, make_shared<AreaLight>(light)};
  GeoPrimitive s2{make_shared<sphere>(&move, &back, false, 1, 0, 0, 0, 0, 0),
                  make_shared<lambert>(spectrum{.2, .7, .8}), nullptr};
  GeoPrimitive s3{make_shared<sphere>(&vertshift, &ov, false, 3, 0, 0, 0, 0, 0),
                  make_shared<lambert>(spectrum{.7, .7, .7}), nullptr};
  GeoPrimitive s4{make_shared<sphere>(&ov, &vertshift, false, 3, 0, 0, 0, 0, 0),
                  make_shared<lambert>(spectrum{0.9, 0.5, 0.5}), nullptr};

  vector<GeoPrimitive> x;
  x.push_back(jaja);
  x.push_back(s2);
  x.push_back(s3);
  x.push_back(s4);
  dumb_aggregate world{x};

  int width = 200;
  int height = 200;
  film f{width, height};
  scene s{{}, std::make_shared<dumb_aggregate>(world)};
  TempCamera cam{f, {0, 0, -5}, {0, 1, 0}, {0, 0, 1}, 1, 0, 90};
  ProgressiveRenderer renderer(s, cam, 10, 1000);

  auto callback = [&](int x) {
    freopen(("frank" + to_string(x) + ".ppm").c_str(), "w", stdout);
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
