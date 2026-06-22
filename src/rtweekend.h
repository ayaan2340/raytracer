#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

#ifdef __CUDACC__
#define RT_HOSTDEV __host__ __device__
#else
#define RT_HOSTDEV
#endif

using Real = float;

constexpr Real infinity = std::numeric_limits<Real>::infinity();
constexpr Real pi = 3.1415926535897932385f;

struct rng {
    uint32_t state;

    RT_HOSTDEV explicit rng(uint32_t seed = 1u) : state(seed ? seed : 1u) {}

    RT_HOSTDEV uint32_t next_u32() {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }

    RT_HOSTDEV Real next_real() {
        return static_cast<Real>(next_u32() >> 8) * (1.0f / 16777216.0f);
    }

    RT_HOSTDEV Real next_real(Real min, Real max) {
        return min + (max - min) * next_real();
    }
};

RT_HOSTDEV inline Real degrees_to_radians(Real degrees) {
    return degrees * pi / 180.0f;
}

RT_HOSTDEV inline uint32_t hash_combine(uint32_t a, uint32_t b) {
    a ^= b + 0x9e3779b9u + (a << 6) + (a >> 2);
    return a;
}

#endif
