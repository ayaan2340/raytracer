#ifndef VEC3_H
#define VEC3_H

#include <array>

class vec3 {
    public:
        std::array<double, 3> e = {0, 0, 0};
        
        constexpr vec3() = default;
        constexpr vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

        constexpr double x() const { return e[0]; }
        constexpr double y() const { return e[1]; }
        constexpr double z() const { return e[2]; }

        constexpr vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
        constexpr double operator[](int i) const { return e[i]; }
        constexpr double& operator[](int i) { return e[i]; }

        constexpr vec3& operator+=(const vec3& v) {
            e[0] += v.e[0];
            e[1] += v.e[1];
            e[2] += v.e[2];
            return *this;
        }

        constexpr vec3& operator-=(const vec3& v) {
            e[0] -= v.e[0];
            e[1] -= v.e[1];
            e[2] -= v.e[2];
            return *this;
        }

        constexpr vec3& operator*=(double t) {
            e[0] *= t;
            e[1] *= t;
            e[2] *= t;
            return *this;
        }

        constexpr vec3& operator/=(double t) {
            return *this *= 1/t;
        }

        constexpr double length() const {
            return std::sqrt(length_squared());
        }

        constexpr double length_squared() const {
            return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
        }

        static vec3 random() {
            return vec3(random_double(), random_double(), random_double());
        }

        static vec3 random(double min, double max) {
            return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
        }

};

using point3 = vec3;

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

constexpr inline vec3 operator+(const vec3& v1, const vec3& v2) {
    return vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}

constexpr inline vec3 operator-(const vec3& v1, const vec3& v2) {
    return vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}

constexpr inline vec3 operator*(const vec3& v, double t) {
    return vec3(v.e[0] * t, v.e[1] * t, v.e[2] * t);
}

constexpr inline vec3 operator*(double t, const vec3& v) {
    return v * t;
}

constexpr inline vec3 operator/(const vec3& v, double t) {
    return (1/t) * v;
}

constexpr inline double dot(const vec3& v1, const vec3& v2) {
    return v1.e[0] * v2.e[0]
         + v1.e[1] * v2.e[1]
         + v1.e[2] * v2.e[2];
}

constexpr inline vec3 cross(const vec3& v1, const vec3& v2) {
    return vec3( v1.e[1] * v2.e[2] - v1.e[2] * v2.e[1], 
                 v1.e[2] * v2.e[0] - v1.e[0] * v2.e[2],
                 v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0]);
}

constexpr inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

inline vec3 random_unit_vector() {
    while (true) {
        vec3 p {vec3::random()};
        auto length_squared {p.length_squared()};
        if (1e-160 < length_squared && length_squared <= 1)
            return p / sqrt(length_squared);
    }
}

inline vec3 random_on_hemisphere(const vec3& normal) {
    vec3 on_unit_sphere {random_unit_vector()};
    if (dot(on_unit_sphere, normal) < 0)
        return -on_unit_sphere;
    return on_unit_sphere;
}

#endif