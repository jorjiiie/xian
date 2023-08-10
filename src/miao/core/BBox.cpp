#include "BBox.hpp"

namespace miao {
bool BBox::intersect(const ray &r, double &t0, double &t1) const {
  cnter_::bbox_tests++;
  double h0 = 0, h1 = r.maxt;
  for (int i = 0; i < 3; i++) {
    double inv_dn = 1.0 / r.d[i];
    double tmn = (mn[i] - r.o[i]) * inv_dn;
    double tmx = (mx[i] - r.o[i]) * inv_dn;
    if (tmn > tmx)
      std::swap(tmn, tmx);

    chmax(h0, tmn);
    chmin(h1, tmx);
    if (h0 > h1)
      return false;
  }
  t0 = h0;
  t1 = h1;
  return true;
}
bool BBox::intersecthuh(const ray &r) const {
  double a;
  return intersect(r, a, a);
}
} // namespace miao
