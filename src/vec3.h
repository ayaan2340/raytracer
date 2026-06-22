#ifndef VEC3_H
#define VEC3_H

#include "rtweekend.h"

#include <array>

class vec3 {
  public:
    std::array<Real, 3> e = {0, 0, 0};

    RT_HOSTDEV constexpr vec3() = default;
    RT_HOSTDEV constexpr vec3(Real e0, Real e1, Real e2) : e{e0, e1, e2} {}

    RT_HOSTDEV constexpr Real x() const { return e[0]; }
    RT_HOSTDEV constexpr Real y() const { return e[1]; }
    RT_HOSTDEV constexpr Real z() const { return e[2]; }

    RT_HOSTDEV constexpr vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    RT_HOSTDEV constexpr Real operator[](int i) const { return e[i]; }
    RT_HOSTDEV constexpr Real& operator[](int i) { return e[i]; }

    RT_HOSTDEV vec3& operator+=(const vec3& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    RT_HOSTDEV vec3& operator-=(const vec3& v) {
        e[0] -= v.e[0];
        e[1] -= v.e[1];
        e[2] -= v.e[2];
        return *this;
    }

    RT_HOSTDEV vec3& operator*=(Real t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    RT_HOSTDEV vec3& operator/=(Real t) { return *this *= 1 / t; }

    RT_HOSTDEV Real length() const { return std::sqrt(length_squared()); }

    RT_HOSTDEV Real length_squared() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    RT_HOSTDEV bool near_zero() const {
        constexpr Real s = 1e-8f;
        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
    }
};

using point3 = vec3;
using color = vec3;

RT_HOSTDEV inline vec3 operator+(const vec3& v1, const vec3& v2) {
    return vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}

RT_HOSTDEV inline vec3 operator-(const vec3& v1, const vec3& v2) {
    return vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}

RT_HOSTDEV inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

RT_HOSTDEV inline vec3 operator*(const vec3& v, Real t) {
    return vec3(v.e[0] * t, v.e[1] * t, v.e[2] * t);
}

RT_HOSTDEV inline vec3 operator*(Real t, const vec3& v) { return v * t; }

RT_HOSTDEV inline vec3 operator/(const vec3& v, Real t) { return (1 / t) * v; }

RT_HOSTDEV inline Real dot(const vec3& v1, const vec3& v2) {
    return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2];
}

RT_HOSTDEV inline vec3 cross(const vec3& v1, const vec3& v2) {
    return vec3(v1.e[1] * v2.e[2] - v1.e[2] * v2.e[1],
                v1.e[2] * v2.e[0] - v1.e[0] * v2.e[2],
                v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0]);
}

RT_HOSTDEV inline vec3 unit_vector(const vec3& v) { return v / v.length(); }

RT_HOSTDEV inline vec3 random_unit_vector(rng& rand) {
    while (true) {
        vec3 p(rand.next_real(-1, 1), rand.next_real(-1, 1), rand.next_real(-1, 1));
        Real length_squared = p.length_squared();
        if (1e-8f < length_squared && length_squared <= 1.0f)
            return p / std::sqrt(length_squared);
    }
}

RT_HOSTDEV inline vec3 random_on_hemisphere(const vec3& normal, rng& rand) {
    vec3 on_unit_sphere = random_unit_vector(rand);
    if (dot(on_unit_sphere, normal) > 0)
        return -on_unit_sphere;
    return on_unit_sphere;
}

RT_HOSTDEV inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2 * dot(v, n) * n;
}

RT_HOSTDEV inline vec3 refract(const vec3& uv, const vec3& n, Real etai_over_etat) {
    Real cos_theta = std::fmin(dot(-uv, n), 1.0f);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0f - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

RT_HOSTDEV inline vec3 random_in_unit_disk(rng& rand) {
    while (true) {
        vec3 p(rand.next_real(-1, 1), rand.next_real(-1, 1), 0);
        if (p.length_squared() < 1)
            return p;
    }
}

#endif
