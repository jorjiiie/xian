#pragma once
#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "miao/core/BBox.hpp"
#include "miao/core/interaction.hpp"
#include "miao/core/ray.hpp"
#include "miao/core/rng.hpp"
#include "miao/core/transform.hpp"
#include "miao/core/utils.hpp"
#include "miao/core/vec3.hpp"

#include <array>
#include <memory>
#include <vector>

namespace miao {
// for now, shapes will be the homebaked solution (but in theory supports
// transforms)
//
vec3 uniform_sphere(RNG &rng);
double uniform_sphere_pdf();

class shape {
public:
  shape(const Transformation *otw, const Transformation *wto, bool reverse)
      : otw(otw), wto(wto), r(reverse) {}

  virtual BBox getBBox() const = 0;

  virtual BBox worldBBox() const { return (*otw)(getBBox()); }

  virtual bool intersect(const ray &r, double &t,
                         SurfaceInteraction &isect) const = 0;

  virtual bool intersecthuh(const ray &r_) const {
    double t = r_.maxt;
    SurfaceInteraction i{};
    return intersect(r_, t, i);
  }

  virtual double area() const = 0;

  // i think these are just rng
  virtual interaction sample(RNG &r) const = 0;
  virtual double pdf(const interaction &) const { return 1.0 / area(); }

  virtual interaction sample(const interaction &, RNG &rng) const {
    return sample(rng);
  }
  virtual double pdf(const interaction &r, const vec3 &wi) const = 0;

  const Transformation *otw, *wto;
  bool r;
};

class sphere : public shape {
public:
  sphere(const Transformation *otw, const Transformation *wto, bool rev,
         double r, double zmin, double zmax, double tmin, double tmax,
         double pmax)
      : shape(otw, wto, rev), radius(r), zmin(clamp(zmin, -r, r)),
        zmax(clamp(zmax, -r, r)), tmin(std::acos(clamp(zmin / r, -1.0, 1.0))),
        tmax(clamp(zmax / r, -1.0, 1.0)), pmax(clamp(pmax, 0.0, 360.0)),
        origin((*otw)(point3{}, true)) {}
  virtual BBox getBBox() const override;
  virtual bool intersect(const ray &r, double &t,
                         SurfaceInteraction &isect) const override;
  virtual double area() const override;
  virtual interaction sample(RNG &r) const override;
  virtual interaction sample(const interaction &i, RNG &r) const override;
  virtual double pdf(const interaction &r, const vec3 &wi) const override;

private:
  double radius;
  double zmin, zmax;
  double tmin, tmax, pmax;
  vec3 origin;
};

class triangle;

// this will read stuff from obj files
struct TriangleMesh {
  static TriangleMesh read_obj(std::istream &str, const Transformation *wto,
                               const Transformation *otw);
  TriangleMesh(const Transformation *otw, const Transformation *wto,
               std::vector<vec3> vertices_, std::vector<vec3> normals_,
               std::vector<vec3> textures_,
               std::vector<std::array<std::array<int, 3>, 3>> faces);
  // three coords for every face!
  std::vector<std::shared_ptr<triangle>> tris;
  std::vector<vec3> v;
  std::vector<vec3> n;
  std::vector<vec3> t;
};

class triangle : public shape {
public:
  // these are identity right?
  triangle(const TriangleMesh *mesh_, std::array<std::array<int, 3>, 3> indexes)
      : shape(&Transformation::identity, &Transformation::identity, false),
        mesh(mesh_), vertices(indexes[0]), normals(indexes[2]),
        texture(indexes[1]) {}
  virtual BBox getBBox() const override;
  virtual BBox worldBBox() const override;
  virtual bool intersect(const ray &r, double &t,
                         SurfaceInteraction &isect) const override;
  virtual double area() const override;
  virtual interaction sample(RNG &r) const override;
  virtual interaction sample(const interaction &i, RNG &r) const override;
  virtual double pdf(const interaction &r, const vec3 &wi) const override;

private:
  vec3 gnormal() const {

    const vec3 &n1 = mesh->n[normals[0]];
    const vec3 &n2 = mesh->n[normals[1]];
    const vec3 &n3 = mesh->n[normals[2]];
    return (n1 + n2 + n3) / 3;
  }
  vec3 snormal(double u, double v) const {

    if (mesh->n.size() == 0)
      return gnormal();
    const vec3 &n1 = mesh->n[normals[0]];
    const vec3 &n2 = mesh->n[normals[1]];
    const vec3 &n3 = mesh->n[normals[2]];
    return (1 - u - v) * n1 + u * n2 + v * n3;
  }
  const TriangleMesh *mesh;
  std::array<int, 3> vertices;
  std::array<int, 3> normals;
  std::array<int, 3> texture;
};

} // namespace miao
#endif
