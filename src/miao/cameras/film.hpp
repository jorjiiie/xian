#pragma once
#ifndef FILM_HPP
#define FILM_HPP

#include "miao/core/spectrum.hpp"

#include <vector>

namespace miao {

class film {
public:
  struct pix {
    double r = 0, g = 0, b = 0;
    pix() {}
    pix(double r, double g, double b) : r(r), g(g), b(b) {}
  };
  film(int width, int height);
  void add_sample(int x, int y, spectrum &s);
  pix pixel(int x, int y) {
    pix ret;
    img[x][y].toRGB(ret.r, ret.g, ret.b);
    return ret;
  }

private:
  std::vector<std::vector<spectrum>> img;
};

} // namespace miao
#endif // FILM_HPP
