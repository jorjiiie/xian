#pragma once
#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "miao/core/BBox.hpp"
#include "miao/core/Matrix4.hpp"
#include "miao/core/debug.hpp"
#include "miao/core/ray.hpp"
#include "miao/core/utils.hpp"
#include "miao/core/vec3.hpp"

namespace miao {
class Transformation {
public:
  Transformation() {}
  Transformation(const double mat[4][4]) : m(mat), m_inv(m.inverse()) {}
  Transformation(const Matrix4 &m_) : m(m_), m_inv(m.inverse()) {}
  Transformation(const Matrix4 &m_, const Matrix4 &m_inv_)
      : m(m_), m_inv(m_inv_) {}

  static Transformation inverse(const Transformation &t) {
    return Transformation{t.m_inv, t.m};
  }

  static Transformation translate(const vec3 &d) {
    Matrix4 m{1, 0, 0, d.x, 0, 1, 0, d.y, 0, 0, 1, d.z, 0, 0, 0, 1};
    Matrix4 m_inv{1, 0, 0, -d.x, 0, 1, 0, -d.y, 0, 0, 1, -d.z, 0, 0, 0, 1};
    return Transformation{m, m_inv};
  }

  static Transformation scale(double x, double y, double z) {
    Matrix4 m{x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1};
    Matrix4 m_inv{1.0 / x, 0, 0,       0, 0, 1.0 / y, 0, 0,
                  0,       0, 1.0 / z, 0, 0, 0,       0, 1.0};
    return Transformation{m, m_inv};
  }

  static Transformation rotateX(double t) {
    double st = std::sin(t / 180.0 * PI);
    double ct = std::cos(t / 180.0 * PI);
    Matrix4 m{1, 0, 0, 0, 0, ct, -st, 0, 0, st, ct, 0, 0, 0, 0, 1};
    return Transformation{m, m.transpose()};
  }

  static Transformation rotateY(double t) {
    double st = std::sin(t / 180.0 * PI);
    double ct = std::cos(t / 180.0 * PI);
    Matrix4 m{ct, 0, st, 0, 0, 1, 0, 0, -st, 0, ct, 0, 0, 0, 0, 1};
    return Transformation{m, m.transpose()};
  }

  static Transformation rotateZ(double t) {
    double st = std::sin(t / 180.0 * PI);
    double ct = std::cos(t / 180.0 * PI);
    Matrix4 m{ct, -st, 0, 0, st, ct, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    return Transformation{m, m.transpose()};
  }

  static Transformation lookAt(const point3 &p, const point3 &look,
                               const point3 &up) {
    Matrix4 ctw;
    ctw.m[0][3] = p.x;
    ctw.m[1][3] = p.y;
    ctw.m[2][3] = p.z;
    ctw.m[3][3] = 1;

    vec3 d = (look - p).unit();
    vec3 right = (vec3::cross(up.unit(), d)).unit();
    vec3 upUnit = vec3::cross(d, right);

    ctw.m[0][0] = right.x;
    ctw.m[1][0] = right.y;
    ctw.m[2][0] = right.z;
    ctw.m[3][0] = 0.;
    ctw.m[0][1] = upUnit.x;
    ctw.m[1][1] = upUnit.y;
    ctw.m[2][1] = upUnit.z;
    ctw.m[3][1] = 0.;
    ctw.m[0][2] = d.x;
    ctw.m[1][2] = d.y;
    ctw.m[2][2] = d.z;
    ctw.m[3][2] = 0.;

    return Transformation{ctw};
  }

  // bool for throwaway LMAO
  point3 operator()(const point3 &p, bool f) const {
    double x = p.x, y = p.y, z = p.z;
    // unroll the multiplication
    double xp = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z + m.m[0][3];
    double yp = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z + m.m[1][3];
    double zp = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z + m.m[2][3];
    double wp = m.m[3][0] * x + m.m[3][1] * y + m.m[3][2] * z + m.m[3][3];
    if (wp == 1)
      return point3{xp, yp, zp};
    else
      return point3{xp, yp, zp} / wp;
  }

  vec3 operator()(const vec3 &v) const {
    double x = v.x, y = v.y, z = v.z;
    // unroll the multiplication
    double xp = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z;
    double yp = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z;
    double zp = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z;
    return vec3{xp, yp, zp};
  }

  // remember that the normal is transformed by the inverse transpose! i'm not
  // implementing that though since i don't have normals lol

  ray operator()(const ray &r) const {
    return ray{(*this)(r.o, true), (*this)(r.d), r.mint, r.maxt, r.time, r.m};
  }

  BBox operator()(const BBox &b) const {
    BBox ret{(*this)(b.mn, true), (*this)(b.mx, true)};
    for (int i = 0; i < 8; i++) {
      ret = ret.Union((*this)(b.corner(i)));
    }
    return ret;
  }

  Transformation operator*(const Transformation &o) const {
    return Transformation(m * o.m, o.m_inv * m_inv);
  }

private:
  Matrix4 m, m_inv;
};

} // namespace miao
#endif // transform_hpp
