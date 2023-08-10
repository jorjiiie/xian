#include "miao/ds/bvh.hpp"
#include "miao/core/interaction.hpp"
#include "miao/core/utils.hpp"

#include "miao/core/debug.hpp"

#include <algorithm>

namespace miao {
bvh::bvh(const std::vector<std::shared_ptr<primitive>> &prims, int maxnode)
    : p(prims), max(maxnode) {
  if (p.size() == 0)
    return;

  std::vector<_bvhinfo> infos(p.size());
  for (size_t i = 0; i < p.size(); i++) {
    infos[i] = {i, p[i]->worldbound()};
  }
  int N = 0;
  _bvhnode_ptr *rt;

  std::vector<std::shared_ptr<primitive>> prim;

  rt = build(infos, 0, p.size(), N, prim);

  std::swap(prim, p);

  nodes_.resize(N);
  int offset = 0;
  _flatten(rt, offset);
}
bvh::_bvhnode_ptr *bvh::build(std::vector<_bvhinfo> &infos, int l, int r,
                              int &tot,
                              std::vector<std::shared_ptr<primitive>> &prims) {
  _bvhnode_ptr *node = new _bvhnode_ptr();
  tot++;
  BBox b = infos[l].b;
  for (int i = l + 1; i < r; i++) {
    b = b.Union(infos[i].b);
  }
  int nprims = r - l;
  if (nprims == 1) {
    // calculate bounds
    int offset = prims.size();
    for (int i = l; i < r; i++) {
      prims.push_back(p[infos[i].idx]);
    }
    node->init_leaf(offset, nprims, b);
    return node;
  }
  BBox cb = infos[l].center;
  for (int i = l; i < r; i++) {
    cb.Union(infos[i].center);
  }
  int d = cb.max_axis();
  if (cb.mx[d] == cb.mn[d]) {
    int offset = prims.size();
    for (int i = l; i < r; i++) {
      prims.push_back(p[infos[i].idx]);
    }
    node->init_leaf(offset, nprims, b);
    return node;
  }

  // do node work
  double median = (cb.mn[d] + cb.mx[d]) / 2;

  const _bvhinfo *mid =
      std::partition(&infos[l], &infos[r - 1] + 1, [d, median](const auto &nd) {
        return nd.center[d] < median;
      });
  int m = mid - &infos[0];

  node->init_int(d, build(infos, l, m, tot, prims),
                 build(infos, m, r, tot, prims));

  return node;
}

int bvh::_flatten(_bvhnode_ptr *ptr, int &offset) {
  _bvhnode &node = nodes_[offset];
  node.b = ptr->bounds;
  int local = offset++;
  if (ptr->nPrims > 0) {
    node.prim_offset = ptr->firstPrimOffset;
    node.nprims = ptr->nPrims;
  } else {
    node.axis = ptr->splitAxis;
    node.nprims = 0;
    _flatten(ptr->child[0], offset);
    node.child_offset = _flatten(ptr->child[1], offset);
  }

  return local;
}

BBox bvh::worldbound() const { return nodes_[0].b; }

std::optional<SurfaceInteraction> bvh::intersect(const ray &r,
                                                 double &t) const {
  std::optional<SurfaceInteraction> ret = {};
  t = D_INFINITY;
  int q[64] = {};
  int qpos = 0;
  int c = 0;
  while (true) {
    DEBUG(qpos, " ", tt, " ", c);
    const _bvhnode &node = nodes_[c];
    if (node.b.intersecthuh(r)) {
      if (node.nprims > 0) {
        for (int i = 0; i < node.nprims; i++) {
          // we don't have ray max unforunately
          double tmp = 0;
          auto y = p[node.prim_offset + i]->intersect(r, tmp);
          if (y && (!ret || t > tmp)) {
            ret = std::move(y), t = tmp;
          }
        }
        if (qpos == 0)
          break;
        else
          c = q[--qpos];
      } else {
        int ax = node.axis;
        if (r.d[ax] < 0) {
          q[qpos++] = c + 1;
          c = node.child_offset;
        } else {
          q[qpos++] = node.child_offset;
          c++;
        }
      }
    } else {
      if (qpos == 0)
        break;
      c = q[--qpos];
    }
  }
  return ret;
}

} // namespace miao
