#include "miao/cameras/camera.hpp"
#include "miao/cameras/film.hpp"
#include "miao/core/material.hpp"
#include "miao/core/primitive.hpp"
#include "miao/core/scene.hpp"
#include "miao/core/shape.hpp"
#include "miao/core/transform.hpp"
#include "miao/core/utils.hpp"
#include "miao/ds/bvh.hpp"
#include "miao/lights/light.hpp"
#include "miao/renderers/progressive.hpp"

#include "miao/core/debug.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;
using namespace miao;

int64_t cnter_::bbox_tests = 0;
int64_t cnter_::prim_tests = 0;
int to_8bit(double d) { return miao::max(miao::min(1.0, d), 0.0) * 255; }

void tmp(film::pix &p, int s) {

  p.r = std::sqrt(p.r / s);
  p.g = std::sqrt(p.g / s);
  p.b = std::sqrt(p.b / s);
}

int main() {

  cout << Transformation::identity.ts() << "\n";
  ifstream str{"models/bunny.obj"};

  Transformation down2 = Transformation::translate({0, -2, -2}) *
                         Transformation::scale(2, 2, 2) *
                         Transformation::rotateY(120);
  Transformation up2 = Transformation::translate({0, 2, 0});
  TriangleMesh mesh = TriangleMesh::read_obj(str, &down2, &up2);
  Transformation id{};
  spectrum li{1.5, 1.5, 1.5};

  vec3 dD{1, 1, 0};
  Transformation tt = Transformation::translate(dD);
  Transformation invtt = Transformation::translate(-dD);
  Transformation left_wall = Transformation::translate(vec3{10005, 0, 0});
  Transformation left_wall_inv = Transformation::translate(vec3{-10005, 0, 0});
  Transformation move_up = Transformation::translate({0, 10005, 0});
  Transformation move_down = Transformation::translate({0, -10005, 0});
  Transformation move_back = Transformation::translate({0, 0, 10005});
  Transformation move_forward = Transformation::translate({0, 0, -10005});

  shared_ptr<material> green_wall =
      make_shared<lambert>(spectrum{0.3, 0.9, 0.5});
  shared_ptr<material> red_wall = make_shared<lambert>(spectrum{0.9, 0.4, 0.4});

  shared_ptr<shape> lc =
      make_shared<sphere>(&tt, &invtt, false, 2, -2, 2, 0, 0, 0);

  shared_ptr<AreaLight> alight = make_shared<AreaLight>(li, lc);

  shared_ptr<material> bs = make_shared<lambert>(spectrum{.5, .5, .5});

  shared_ptr<material> gl =
      make_shared<glass>(spectrum{1.0, 1.0, 1.0}, 1.0, 1.5);

  GeoPrimitive LW{make_shared<sphere>(&left_wall, &left_wall_inv, false, 10000,
                                      0, 0, 0, 0, 0),
                  red_wall, nullptr};
  GeoPrimitive RW{make_shared<sphere>(&left_wall_inv, &left_wall, false, 10000,
                                      0, 0, 0, 0, 0),
                  green_wall, nullptr};
  GeoPrimitive CEIL{
      make_shared<sphere>(&move_up, &move_down, false, 10000, 0, 0, 0, 0, 0),
      bs, nullptr};
  GeoPrimitive FLOOR{
      make_shared<sphere>(&move_down, &move_up, false, 10000, 0, 0, 0, 0, 0),
      bs, nullptr};

  GeoPrimitive BACK_WALL{make_shared<sphere>(&move_back, &move_forward, false,
                                             10000, 0, 0, 0, 0, 0),
                         bs, nullptr};
  shared_ptr<material> spec = make_shared<metal>(spectrum{.5, 0.9, 0.7});
  Transformation vertshift = Transformation::translate({-1, 5, 0});
  Transformation ov = Transformation::translate({1, -5, 0});

  Transformation move = Transformation::translate({-1, -2, 1});
  Transformation back = Transformation::translate({1, 2, -1});

  GeoPrimitive jaja{lc, bs, alight};
  GeoPrimitive s2{make_shared<sphere>(&move, &back, false, 1, -1, 1, 0, 0, 0),
                  gl, nullptr};
  // make_shared<lambert>(spectrum{.2, .7, .8}), nullptr};
  GeoPrimitive s3{
      make_shared<sphere>(&vertshift, &ov, false, 3, -3, 3, 0, 0, 0), spec,
      nullptr};
  GeoPrimitive s4{
      make_shared<sphere>(&ov, &vertshift, false, 3, -3, 3, 0, 0, 0),
      make_shared<lambert>(spectrum{0.9, 0.5, 0.5}), nullptr};
  GeoPrimitive s5{make_shared<sphere>(&id, &id, false, 8, 0, 0, 0, 0, 0), bs,
                  nullptr};

  vector<GeoPrimitive> tris;
  for (auto &x : mesh.tris) {
    tris.push_back(GeoPrimitive{x, bs, nullptr});
  }

  vector<GeoPrimitive> x;
  x.push_back(jaja);
  x.push_back(s2);
  x.push_back(s3);
  // x.push_back(s4);
  //  x.push_back(s5);
  x.push_back(LW);
  x.push_back(RW);
  x.push_back(CEIL);
  x.push_back(FLOOR);
  x.push_back(BACK_WALL);
  for (auto &y : tris) {
    x.push_back(y);
  }
  dumb_aggregate da{x};
  vector<shared_ptr<primitive>> arr;
  for (auto &z : x) {
    arr.push_back(make_shared<GeoPrimitive>(z));
  }
  bvh world{arr};

  vector<shared_ptr<light>> lights;
  lights.push_back(alight);

  int width = 500;
  int height = 500;
  film f{width, height};
  scene s{lights, std::make_shared<bvh>(world)};
  // scene s{lights, std::make_shared<dumb_aggregate>(da)};

  TempCamera cam{f, {0, 0, -8}, {0, 1, 0}, {0, 0, 1}, 1, 0, 90};
  ProgressiveRenderer renderer(s, cam, 20, 10);

  auto callback = [&](int x) {
    freopen(("nn" + to_string(x) + ".ppm").c_str(), "w", stdout);
    std::cerr << "bbox tests: " << cnter_::bbox_tests << " "
              << " primitive tests: " << cnter_::prim_tests << "\n";
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
