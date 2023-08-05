#pragma once
#ifndef RNG_HPP
#define RNG_HPP

#include <cstdint>
#include <iostream>

namespace miao {
const int64_t PCG32_DEFAULT_STATE = 0x853c49e6748fea9bULL;
const int64_t PCG32_DEFAULT_STREAM = 0xda3e39cb94b95bdbULL;
const int64_t PCG32_MULT = 0x5851f42d4c957f2dULL;

// probably bad performance
struct RNG {
  virtual uint32_t rint() = 0;
  virtual float rfloat() = 0;
};

// https://github.com/wjakob/pcg32/blob/master/pcg32.h
class pcg32 : public RNG {
public:
  pcg32() : state(PCG32_DEFAULT_STATE), inc(PCG32_DEFAULT_STREAM) {}

  /// Initialize the pseudorandom number generator with the \ref seed()
  /// function
  pcg32(uint64_t initstate, uint64_t initseq = 1u) { seed(initstate, initseq); }

  /**
   * \brief Seed the pseudorandom number generator
   *
   * Specified in two parts: a state initializer and a sequence selection
   * constant (a.k.a. stream id)
   */
  void seed(uint64_t initstate, uint64_t initseq = 1) {

    state = 0U;
    inc = (initseq << 1u) | 1u;
    rint();
    state += initstate;
    rint();
  }
  /// Generate a uniformly distributed unsigned 32-bit random number
  uint32_t rint() {
    uint64_t oldstate = state;
    state = oldstate * PCG32_MULT + inc;
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = (uint32_t)(oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
  }

  /// Generate a uniformly distributed number, r, where 0 <= r < bound
  uint32_t nextUInt(uint32_t bound) {
    // To avoid bias, we need to make the range of the RNG a multiple of
    // bound, which we do by dropping output less than a threshold.
    // A naive scheme to calculate the threshold would be to do
    //
    //     uint32_t threshold = 0x100000000ull % bound;
    //
    // but 64-bit div/mod is slower than 32-bit div/mod (especially on
    // 32-bit platforms).  In essence, we do
    //
    //     uint32_t threshold = (0x100000000ull-bound) % bound;
    //
    // because this version will calculate the same modulus, but the LHS
    // value is less than 2^32.

    uint32_t threshold = (~bound + 1u) % bound;

    // Uniformity guarantees that this loop will terminate.  In practice, it
    // should usually terminate quickly; on average (assuming all bounds are
    // equally likely), 82.25% of the time, we can expect it to require just
    // one iteration.  In the worst case, someone passes a bound of 2^31 + 1
    // (i.e., 2147483649), which invalidates almost 50% of the range.  In
    // practice, bounds are typically small and only a tiny amount of the range
    // is eliminated.
    for (;;) {
      uint32_t r = rint();
      if (r >= threshold)
        return r % bound;
    }
  }

  /// Generate a single precision floating point value on the interval [0, 1)
  float rfloat() {
    /* Trick from MTGP: generate an uniformly distributed
       single precision number in [1,2) and subtract 1. */
    union {
      uint32_t u;
      float f;
    } x;
    x.u = (rint() >> 9) | 0x3f800000u;
    return x.f - 1.0f;
  }

  /**
   * \brief Generate a double precision floating point value on the interval [0,
   * 1)
   *
   * \remark Since the underlying random number generator produces 32 bit
   * output, only the first 32 mantissa bits will be filled (however, the
   * resolution is still finer than in \ref nextFloat(), which only uses 23
   * mantissa bits)
   */
  double nextDouble() {
    /* Trick from MTGP: generate an uniformly distributed
       double precision number in [1,2) and subtract 1. */
    union {
      uint64_t u;
      double d;
    } x;
    x.u = ((uint64_t)rint() << 20) | 0x3ff0000000000000ULL;
    return x.d - 1.0;
  }

private:
  uint64_t state; // RNG state.  All values are possible.
  uint64_t inc;   // Controls which RNG sequence (stream) is selected. Must
                  // *always* be odd.
};
} // namespace miao

#endif // rng_hpp
