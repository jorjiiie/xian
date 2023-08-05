#pragma once
#ifndef VEC3_HPP
#define VEC3_HPP

#include <cmath>
#include <limits>
#include <string>

#include "utils.hpp"

namespace miao {

struct vec3 {
  vec3() : x(0), y(0), z(0) {}
  vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
  vec3(const vec3 &v) : x(v.x), y(v.y), z(v.z) {}

  // element wise operators
  vec3 &operator+=(const vec3 &v) {
    this->x += v.x;
    this->y += v.y;
    this->z += v.z;
    return *this;
  }
  vec3 operator+(const vec3 &v) const {
    vec3 n = *this;
    return n += v;
  }
  vec3 &operator-=(const vec3 &v) {
    this->x -= v.x;
    this->y -= v.y;
    this->z -= v.z;
    return *this;
  }
  vec3 operator-(const vec3 &v) const {
    vec3 n = *this;
    return n -= v;
  }
  vec3 &operator*=(const vec3 &v) {
    this->x *= v.x;
    this->y *= v.y;
    this->z *= v.z;
    return *this;
  }
  vec3 operator*(const vec3 &v) const {
    vec3 n = *this;
    return n *= v;
  }

  // unary operators
  vec3 operator-() const { return vec3(-this->x, -this->y, -this->z); }

  // scalar operations
  vec3 operator*(double r) const {
    return vec3(this->x * r, this->y * r, this->z * r);
  }
  vec3 &operator*=(double r) {
    this->x *= r;
    this->y *= r;
    this->z *= r;
    return *this;
  }

  vec3 operator/(double r) const {
    return vec3(this->x / r, this->y / r, this->z / r);
  }
  vec3 &operator/=(double r) {
    this->x /= r;
    this->y /= r;
    this->z /= r;
    return *this;
  }

  // loop accessor (make a macro for speed)
  double operator[](int i) const {
    switch (i) {
    case 0:
      return this->x;
    case 1:
      return this->y;
    case 2:
      return this->z;
    }
    return 0;
  }
  bool operator==(const vec3 &b) const {
    return (std::abs(x - b.x) < EPS && std::abs(y - b.y) < EPS &&
            std::abs(z - b.z) < EPS);
  }
  bool operator!=(const vec3 &b) const { return !((*this) == b); }

  // misc operators
  static double dot(const vec3 &a, const vec3 &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }
  static vec3 cross(const vec3 &a, const vec3 &b) {
    return vec3(a.y * b.z - a.z * b.y, b.x * a.z - a.x * b.z,
                a.x * b.y - a.y * b.x);
  }
  double magnitude() const { return std::sqrt(dot(*this, *this)); }
  double msq() const { return dot(*this, *this); }
  vec3 &norm() {
    *this *= 1.0 / this->magnitude();
    return *this;
  }
  vec3 unit() const {
    vec3 n = *this;
    return n.norm();
  }

  std::string ts() const {
    return (std::string{"["} + std::to_string(x) + std::string{", "} +
            std::to_string(y) + std::string{", "} + std::to_string(z) +
            std::string{"]"});
  }

  double x, y, z;
};

vec3 operator*(double d, const vec3 &v) { return v * d; }
vec3 &operator*=(double d, vec3 &v) { return v *= d; }

typedef vec3 point3;
} // namespace miao
#endif // vec3_hpp
