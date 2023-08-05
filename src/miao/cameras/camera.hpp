#pragma once
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "core/ray.hpp"

class camera {
public:
  camera(const Transformation &ctw);
  virtual double gen_ray(RNG &rng, ray &r) const = 0;
};
#endif // CAMERA_HPP
