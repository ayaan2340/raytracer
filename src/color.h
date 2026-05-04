#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

using color = vec3;

double linear_to_gamma(double c) {
    if (c > 0)
        return std::sqrt(c);
    return 0;
}

void write_color(std::ostream &out, const color& pixel) {
    auto r = linear_to_gamma(pixel.x());
    auto g = linear_to_gamma(pixel.y());
    auto b = linear_to_gamma(pixel.z());

    interval intensity = interval(0, 0.999);
    int rbyte = int(256 * intensity.clamp(r));
    int gbyte = int(256 * intensity.clamp(g));
    int bbyte = int(256 * intensity.clamp(b));

    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif