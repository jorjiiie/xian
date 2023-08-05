#pragma once
#ifndef MATRIX4_HPP
#define MATRIX4_HPP

#include <algorithm>
#include <cmath>

#include "utils.hpp"

namespace miao {
struct Matrix4 {
  // init to identity
  Matrix4() { m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1; }

  Matrix4(double a1, double a2, double a3, double a4, double b1, double b2,
          double b3, double b4, double c1, double c2, double c3, double c4,
          double d1, double d2, double d3, double d4) {
    m[0][0] = a1;
    m[0][1] = a2;
    m[0][2] = a3;
    m[0][3] = a4;

    m[1][0] = b1;
    m[1][1] = b2;
    m[1][2] = b3;
    m[1][3] = b4;

    m[2][0] = c1;
    m[2][1] = c2;
    m[2][2] = c3;
    m[2][3] = c4;

    m[3][0] = d1;
    m[3][1] = d2;
    m[3][2] = d3;
    m[3][3] = d4;
  }
  Matrix4(const double m_[4][4]) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        m[i][j] = m_[i][j];
  }

  Matrix4(const Matrix4 &m) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        this->m[i][j] = m.m[i][j];
  }

  Matrix4 &operator*=(const Matrix4 &b) {
    double t[4][4] = {};
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 4; k++) {
          t[i][j] += m[i][k] * b.m[k][j];
        }
      }
    }
    std::swap(m, t);
    return *this;
  }
  Matrix4 operator*(const Matrix4 &b) const {
    Matrix4 t = *this;
    t *= b;
    return t;
  }

  Matrix4 &operator*=(double d) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        m[i][j] *= d;
    return *this;
  }
  Matrix4 operator*(double d) const {
    Matrix4 t = *this;
    t *= d;
    return t;
  }

  Matrix4 &invert();
  Matrix4 transpose() const {
    return Matrix4{m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1],
                   m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2],
                   m[0][3], m[1][3], m[2][3], m[3][3]};
  }

  Matrix4 inverse() const {
    Matrix4 t = *this;
    t.invert();
    return t;
  }

  double m[4][4] = {};
};
} // namespace miao
#endif // matrix4_hpp
