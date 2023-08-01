#pragma once
#ifndef BBOX_HPP
#define BBOX_HPP

#include "vec3.hpp"

struct BBox {
  BBox()
      : mn(vec3{-D_INFINITY, -D_INFINITY, -D_INFINITY}),

        mx(vec3{D_INFINITY, D_INFINITY, D_INFINITY}) {}

  BBox(const point3 &pt) : mn(pt), mx(pt) {}
  BBox(const point3 &a, const point3 &b)
      : mn(vec3{min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)}),
        mx({vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z))}) {}

  BBox &Union(const point3 &p) {
    chmin(mn.x, p.x);
    chmin(mn.y, p.y);
    chmin(mn.z, p.z);
    chmax(mx.x, p.x);
    chmax(mx.y, p.y);
    chmax(mx.z, p.z);
    return *this;
  }

  BBox &Union(const BBox &o) { return (this->Union(o.mn).Union(o.mx)); }
  static BBox Union(const BBox &b, const point3 &p) {
    BBox r = b;
    return r.Union(p);
  }
  static BBox Union(const BBox &a, const BBox &b) {
    auto tmp = a;
    return tmp.Union(b);
  }

  bool overlap(const BBox &b) const {
    return (mn.x <= b.mx.x && b.mn.x <= mx.x) &&
           (mn.y <= b.mx.y && b.mn.y <= mx.y) &&
           (mn.z <= b.mx.z && b.mn.z <= mx.z);
  }

  bool inside(const point3 &p) const {
    return (mn.x <= p.x && p.x <= mx.x) && (mn.y <= p.y && p.y <= mx.y) &&
           (mn.z <= p.z && p.z <= mx.z);
  }

  void expand(double d) {
    mn -= vec3{d, d, d};
    mx += vec3{d, d, d};
  }

  double vol() const { return (mx.x - mn.x) * (mx.y - mn.y) * (mx.z - mn.z); }
  double sa() const {
    vec3 d = mx - mn;
    return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
  }

  int max_axis() const {
    vec3 d = mx - mn;
    if (d.x > d.y && d.x > d.z)
      return 0;
    if (d.y > d.z)
      return 1;
    return 2;
  }

  // missing some from the bottom of the section

  point3 &operator[](int i) {
    if (i == 0)
      return mn;
    return mx;
  }
  const point3 &operator[](int i) const {
    if (i == 0)
      return mn;
    return mx;
  }

  point3 corner(int i) const {
    return point3((*this)[(i & 1)].x, (*this)[(i & 2) ? 1 : 0].y,
                  (*this)[(i & 4) ? 1 : 0].z);
  }
  point3 mn, mx;
};

#endif
