#pragma once
#ifndef INT_COMMON_HPP
#define INT_COMMON_HPP

#include "miao/core/spectrum.hpp"

namespace miao {

class interaction;
class RNG;
class scene;
class light;

spectrum sample_light(const interaction &it, const scene &scene, RNG &rng);

spectrum direct(const interaction &it, const light &li, const scene &s,
                RNG &rng);

// balance heuristic for 2 sampling types
double bh(int lsamp, double lpdf, int ssamp, double spdf);

} // namespace miao

#endif // INT_COMMON_HPP
