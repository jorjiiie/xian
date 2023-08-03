#include "shape.hpp"

BBox sphere::getBBox() const { return BBox{vec3{-r, -r, -r}, vec3{r, r, r}}; }
bool sphere::intersect(const ray &r, double &t,
                       SurfaceInteraction &isect) const {
  double phi, theta;
  point3 hit;
  ray ry = (*wto)(r);
}
bool sphere::intersecthuh(const ray &r) const {}
double sphere::area() const {}
interaction sphere::sample(RNG &r) const {}
interaction sphere::sample(const interaction &i, RNG &r) const {}
double sphere::pdf(const interaction &r, const vec3 &wi) const {}
