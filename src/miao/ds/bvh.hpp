#pragma once
#ifndef BVH_HPP
#define BVH_HPP

#include "miao/core/interaction.hpp"
#include "miao/core/primitive.hpp"

#include <memory>
#include <vector>

namespace miao {

// implements a BVH using the middle of the longest axis
class bvh : public aggregate {
public:
  bvh(const std::vector<std::shared_ptr<primitive>> &prims, int maxnode = 1);
  virtual BBox worldbound() const override;
  virtual std::optional<SurfaceInteraction> intersect(const ray &r,
                                                      double &t) const override;

private:
  std::vector<std::shared_ptr<primitive>> p;
  int max;

  struct _bvhinfo;
  struct _bvhnode_ptr;
  struct _bvhnode;

  _bvhnode_ptr *build(std::vector<_bvhinfo> &infos, int l, int r, int &tot,
                      std::vector<std::shared_ptr<primitive>> &prims);
  int _flatten(_bvhnode_ptr *ptr, int &offset);

  struct _bvhinfo {
    _bvhinfo() {}
    _bvhinfo(size_t i, const BBox &box_)
        : idx(i), b(box_), center(box_.mn * 0.5 + box_.mx * 0.5) {}
    size_t idx;
    BBox b;
    point3 center;
  };
  struct _bvhnode_ptr {
    void init_leaf(int first, int n, const BBox &b) {
      firstPrimOffset = first;
      nPrims = n;
      bounds = b;
      child[0] = child[1] = nullptr;
    }
    void init_int(int axis, _bvhnode_ptr *l, _bvhnode_ptr *r) {
      child[0] = l;
      child[1] = r;
      bounds = l->bounds.Union(r->bounds);
      splitAxis = axis;
      nPrims = 0;
    }
    BBox bounds;
    _bvhnode_ptr *child[2] = {};
    int splitAxis, firstPrimOffset, nPrims;
  };
  struct _bvhnode { // we use doubles in BBOX LOL definitely not padded :)
    BBox b;
    int prim_offset;
    int child_offset;
    int nprims;
    int axis;
  };

  std::vector<_bvhnode> nodes_;
};

} // namespace miao
#endif // BVH_HPP
