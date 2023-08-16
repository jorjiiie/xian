#include "shape.hpp"
#include "miao/core/interaction.hpp"
#include "miao/core/transform.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace miao {

vec3 uniform_sphere(RNG &rng) {
  double z = 1 - 2 * rng.rfloat();
  double r = std::sqrt(max(0.0, 1 - z * z));
  double phi = 2 * PI * rng.rfloat();
  return vec3{r * std::cos(phi), r * std::sin(phi), z};
}
inline double uniform_sphere_pdf() { return INV_PI / 4; }

BBox sphere::getBBox() const {
  return BBox{vec3{-radius, -radius, -radius}, vec3{radius, radius, radius}};
}

// t is outgoing time
bool sphere::intersect(const ray &r, double &t,
                       SurfaceInteraction &isect) const {
  // double phi, theta;
  point3 hit;
  vec3 oc = r.o - origin;

  double a = r.d.msq();
  double b = 2 * vec3::dot(oc, r.d);
  double c = oc.msq() - radius * radius;
  double t0, t1;
  if (!quadratic(a, b, c, t0, t1))
    return false;
  if (t0 > r.maxt || t1 <= 0)
    return false;

  double T = t0;
  if (T < 0) {
    T = t1;
    if (T > r.maxt)
      return false;
  }
  vec3 hp = r.o + T * r.d;
  isect.p = hp;
  isect.wo = r.d;
  isect.n = (isect.p - origin) / radius;
  isect.sn = isect.n;

  isect.t = T;

  t = T;
  return true;

  // for now we just don't care about the parametric stuff
}
double sphere::area() const {
  // compute based off of bounds!
  // integral of 1 from tmin to tmax, 0 to phimax sinT dtdp
  return 4 * PI * radius; // naive no bounds
}
interaction sphere::sample(RNG &rng) const {
  vec3 v = uniform_sphere(rng);
  point3 point = radius * v;
  SurfaceInteraction it;
  it.n = v;
  it.p = point + origin;
  return it;
}
interaction sphere::sample(const interaction &, RNG &r) const {
  return sample(r);
}
double sphere::pdf(const interaction &r, const vec3 &) const {
  return shape::pdf(r);
}

TriangleMesh TriangleMesh::read_obj(std::istream &str,
                                    const Transformation *otw,
                                    const Transformation *wto) {

  std::vector<vec3> vertices;
  std::vector<vec3> normals;
  std::vector<vec3> textures;
  std::vector<std::array<std::array<int, 3>, 3>> faces;
  std::string cur_line;
  while (!str.eof()) {
    std::getline(str, cur_line);
    if (cur_line.size() == 0 || cur_line[0] == '#')
      continue;
    std::istringstream ss(cur_line);
    std::string type;
    ss >> type;
    if (type == "v") {
      double a, b, c;
      ss >> a >> b >> c; // ignores w
      vertices.push_back(vec3{a, b, c});
    } else if (type == "vn") {
      double a, b, c;
      ss >> a >> b >> c;
      normals.push_back(vec3{a, b, c});
    } else if (type == "f") {
      std::regex r("/");
      std::string a, x, y, z;
      std::array<std::array<int, 3>, 3> indices;
      for (int i = 0; i < 3; i++) {
        ss >> a;
        std::sregex_token_iterator iter(a.begin(), a.end(), r, -1);
        std::sregex_token_iterator end;
        int j = 0;
        for (; iter != end && j < 3; iter++, j++) {
          std::string tok = *iter;
          if (tok.length() == 0) {
            indices[j][i] = -1;
            continue;
          }
          indices[j][i] = std::stoi(tok) - 1;
          ASSERT(indices[j][i] >= 0, "invalid indexes!");
        }
      }
      faces.push_back(indices);
    }
  }
  DEBUG(faces.size(), " ", vertices.size(), " ", normals.size(), " ",
        textures.size());
  // exit(1);
  return TriangleMesh(otw, wto, vertices, normals, textures, faces);
}

TriangleMesh::TriangleMesh(const Transformation *otw, const Transformation *,
                           std::vector<vec3> vertices_,
                           std::vector<vec3> normals_,
                           std::vector<vec3> textures_,
                           std::vector<std::array<std::array<int, 3>, 3>> faces)
    : v(vertices_), n(normals_), t(textures_) {

  for (vec3 &vec : v) {
    vec = (*otw)(vec, true); // these are points
  }

  for (vec3 &vec : n)
    vec = (*otw)(vec, 1); // normals

  for (auto &z : faces) {
    std::string str = "";
    for (int i = 0; i < 3; i++) {
      str += std::to_string(z[i][0]);
      str += " ";
      str += std::to_string(z[i][1]);
      str += " ";
      str += std::to_string(z[i][2]);
      str += " || ";
    }
    DEBUG(str);
    DEBUG(v[z[0][0]].ts(), v[z[0][1]].ts(), v[z[0][2]].ts());

    tris.push_back(std::make_shared<triangle>(this, z));
  }
  DEBUG(v.size(), " ", n.size(), " ", tris.size());
  // exit(1);
}

BBox triangle::getBBox() const {
  const vec3 &v1 = mesh->v[vertices[0]];
  const vec3 &v2 = mesh->v[vertices[1]];
  const vec3 &v3 = mesh->v[vertices[2]];
  DEBUG(v1.ts(), v2.ts(), v3.ts());
  return BBox{v1, v2}.Union(v3);
}
BBox triangle::worldBBox() const { return getBBox(); }

bool triangle::intersect(const ray &r, double &t,
                         SurfaceInteraction &isect) const {
  vec3 e1, e2, h, s, q;
  double a, f, u, v;

  const vec3 &v1 = mesh->v[vertices[0]];
  const vec3 &v2 = mesh->v[vertices[1]];
  const vec3 &v3 = mesh->v[vertices[2]];

  e1 = v2 - v1;
  e2 = v3 - v1;
  h = vec3::cross(r.d, e2);
  a = vec3::dot(e1, h);

  if (std::abs(a) < vec3::eps)
    return {};

  f = 1.0 / a;
  s = r.o - v1;
  u = f * vec3::dot(s, h);
  if (u < 0 || u > 1.0)
    return {};

  q = vec3::cross(s, e1);
  v = f * vec3::dot(r.d, q);
  if (v < 0 || u + v > 1.0)
    return {};

  double tt = f * vec3::dot(e2, q);
  if (tt < vec3::eps)
    return {};
  t = tt;

  isect.p = r.o + t * r.d;
  isect.wo = r.d;
  isect.n = gnormal();
  isect.sn = snormal(u, v);
  isect.t = t;

  DEBUG("INTERSECTED A TRIANGLE WHATAFUCK");

  return true;
}

double triangle::area() const {
  const vec3 &v1 = mesh->v[vertices[0]];
  const vec3 &v2 = mesh->v[vertices[1]];
  const vec3 &v3 = mesh->v[vertices[2]];
  return 0.5 * std::abs(vec3::cross(v2 - v1, v3 - v1).magnitude());
}

interaction triangle::sample(RNG &) const { return interaction{}; }
interaction triangle::sample(const interaction &, RNG &) const {
  return interaction{};
}
double triangle::pdf(const interaction &, const vec3 &) const { return 0; }

} // namespace miao
