#include "miao/core/scene.hpp"
#include "miao/core/interaction.hpp"
#include "miao/core/material.hpp"
#include "miao/core/shape.hpp"
#include "miao/core/transform.hpp"
#include "miao/core/utils.hpp"

#include "miao/ds/bvh.hpp"

#include "miao/lights/light.hpp"

#include <array>
#include <tiny_obj_loader.h>
#include <vector>

namespace miao {

std::optional<SurfaceInteraction>
scene::intersectTr(const ray &r, double t, spectrum &tr, RNG &rng) const {
  tr = spectrum{1.0};

  ray x = r;
  for (int i = 0; i < 100; i++) {
    auto y = intersect(x, t);
    if (!y)
      return {}; // this is only because of the lack of infinite lights and
                 // environment maps
    // return the transmittance
    if (x.m != nullptr)
      tr *= x.m->tr(x, y->t, rng);

    if (y->pr->get_material() != nullptr)
      return y;

    x.m = y->get_medium(x.d);
    DEBUG("WHAT\n");
  }
  return {};
}

bool visibility::visible(const scene &s) const {
  vec3 d = (p1.p - p0.p);
  double maxt = d.magnitude();
  ray r{p0.p + (d / maxt * EPS), d / maxt, 0};

  cnter_::rays_cast++;

  auto y = s.intersect(r, 0);
  if (y) {
    return !(y->t + EPS < maxt);
  }
  return true;
}

spectrum visibility::tr(const scene &s, RNG &rng) const {
  vec3 d = (p1.p - p0.p);
  double maxt = d.magnitude();
  ray r{p0.p + (d / maxt * EPS), d / maxt, 0};
  cnter_::rays_cast++;
  spectrum tr{1.0};

  while (true) {
    auto y = s.intersect(r, 0);
    if (!y) {
      if (r.m)
        return r.m->tr(r, D_INFINITY - 1, rng);
      return tr;
    }

    if (y && y->pr->get_material() != nullptr) {
      if (y->t + vec3::eps > maxt)
        return tr;
      return spectrum{0};
    }
    r.maxt = y->t;
    if (r.m != nullptr) {
      tr *= r.m->tr(r, y->t, rng);
    }
    r.maxt = D_INFINITY;

    // continue;
    r.o = r.o + r.d * y->t;
    r.m = y->get_medium(r.d);
    DEBUG("HI", r.o.ts(), " ", y->t, " ", r.d.ts(), " ");
  }
}

// adapted from volppm
bool scene::load_from_obj(const std::string &path) {
  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = "./models/";
  reader_config.triangulate = true;

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path, reader_config)) {
    if (!reader.Error().empty()) {
      FAIL("FAILED TO LOAD FILE " + reader.Error());
    }
    return false;
  }
  if (!reader.Warning().empty()) {
    std::cerr << "WARNING " << reader.Warning();
  }
  const auto &attrib = reader.GetAttrib();
  const auto &shapes = reader.GetShapes();
  const auto &mats = reader.GetMaterials();

  // attrib
  std::vector<vec3> vertices;
  std::vector<vec3> normals;
  std::vector<vec3> texcoords;
  std::vector<std::shared_ptr<material>> materials;

  std::vector<int> material_index;
  std::vector<std::array<std::array<int, 3>, 3>> indices;

  for (size_t i = 0; i < attrib.vertices.size() / 3; i++) {
    auto vx = attrib.vertices[3 * i + 0];
    auto vy = attrib.vertices[3 * i + 1];
    auto vz = attrib.vertices[3 * i + 2];
    vertices.push_back(vec3{vx, vy, vz});
  }
  for (size_t i = 0; i < attrib.normals.size() / 3; i++) {
    auto nx = attrib.normals[3 * i + 0];
    auto ny = attrib.normals[3 * i + 1];
    auto nz = attrib.normals[3 * i + 2];
    normals.push_back(vec3{nx, ny, nz});
  }
  for (size_t i = 0; i < attrib.texcoords.size() / 3; i++) {
    auto tx = attrib.texcoords[3 * i + 0];
    auto ty = attrib.texcoords[3 * i + 1];
    auto tz = attrib.texcoords[3 * i + 2];
    texcoords.push_back(vec3{tx, ty, tz});
  }
  for (size_t i = 0; i < mats.size(); i++) {
    // create a mateiral based off of the material type
    const tinyobj::material_t &mat = mats[i];

    std::cerr << "material! " << mat.diffuse[0] << " " << mat.diffuse[1] << " "
              << mat.diffuse[2] << "\n";
    switch (mat.illum) {
    case 5:
      std::cerr << " metal\n";
      materials.push_back(std::make_shared<metal>(spectrum{1.0}));
      break;
    case 7:
      // glass
      std::cerr << "joe glass\n";
      materials.push_back(std::make_shared<glass>(spectrum{1.0}, 1.0, mat.ior));
      break;
    default:
      // diffuse
      std::cerr << "lambert D:\n";
      materials.push_back(std::make_shared<lambert>(
          spectrum{mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]}));
      break;
    }
  }
  std::cerr << "what is going on  " << materials.size() << " " << mats.size()
            << " mat size\n";

  for (size_t i = 0; i < shapes.size(); i++) {
    size_t offset = 0;
    std::cerr << "uwu\n";
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      const size_t fv =
          static_cast<size_t>(shapes[i].mesh.num_face_vertices[f]);
      std::array<std::array<int, 3>, 3> index;
      // vertices per face!
      for (size_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[offset + v];

        index[0][v] = idx.vertex_index;
        index[2][v] = idx.normal_index;
        index[1][v] = idx.texcoord_index;
      }

      material_index.push_back(shapes[i].mesh.material_ids[f]);
      offset += fv;
      indices.push_back(index);
    }
  }

  triangles = std::make_unique<TriangleMesh>(
      &Transformation::identity, &Transformation::identity, vertices, normals,
      texcoords, indices);
  const TriangleMesh &mesh = *triangles;

  std::vector<std::shared_ptr<primitive>> prims;
  std::cerr << vertices.size() << " " << normals.size() << " "
            << texcoords.size() << " " << indices.size() << "\n";

  int i = 0;
  for (auto &x : mesh.tris) {
    std::shared_ptr<material> _mat;
    if (material_index[i] == -1) {
      _mat = std::make_shared<lambert>(spectrum{0.9, 0.3, 0.4});
    } else {
      _mat = materials[material_index[i]];
    }
    i++;
    prims.push_back(
        std::make_shared<GeoPrimitive>(x, _mat, nullptr, MediumInterface{}));
  }

  // spectrum li{20 / 10, 10 / 10, 6 / 10};
  spectrum li{2.5, 2.5, 2.5};

  // Medium med{spectrum{}}
  vec3 dD{0, 2, 0};
  Transformation tt = Transformation::translate(dD);
  Transformation invtt = Transformation::translate(-dD);
  std::shared_ptr<shape> lc =
      std::make_shared<sphere>(&tt, &invtt, false, .1, -2, 2, 0, 0, 0);

  std::shared_ptr<AreaLight> alight = std::make_shared<AreaLight>(li, lc);

  prims.push_back(std::make_shared<GeoPrimitive>(
      lc, std::make_shared<lambert>(spectrum{1.0}), alight));
  lights.push_back(alight);
  /* prims.push_back(std::make_shared<GeoPrimitive>( */
  /*     std::make_shared<sphere>(&Transformation::identity, */
  /*                              &Transformation::identity, false, 15, 0, 0, 0,
   * 0, */
  /*                              0), */
  /*     std::make_shared<lambert>(spectrum{0.5, 0.5, 0.5}), nullptr)); */
  std::cerr << "loaded " << prims.size() << " primitives!" << std::endl;
  agg = std::make_shared<bvh>(prims);
  return true;
}
} // namespace miao
