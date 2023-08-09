#include "miao/cameras/film.hpp"

namespace miao {

film::film(int width, int height) : width(width), height(height) {
  img.resize(width, std::vector<std::pair<spectrum, int>>(
                        height, std::make_pair(spectrum{}, 0)));
}
void film::add_sample(int x, int y, spectrum &s, int cnt) {

  img[x][y].first += s;
  img[x][y].second += cnt;
}
} // namespace miao
