#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "interval.h"

#include <cstdint>
#include <vector>

RT_HOSTDEV inline Real linear_to_gamma(Real c) {
    if (c > 0)
        return std::sqrt(c);
    return 0;
}

inline void color_to_bytes(const color& pixel, uint8_t& r, uint8_t& g, uint8_t& b) {
    Real r_lin = linear_to_gamma(pixel.x());
    Real g_lin = linear_to_gamma(pixel.y());
    Real b_lin = linear_to_gamma(pixel.z());

    interval intensity(0, 0.999f);
    r = static_cast<uint8_t>(256 * intensity.clamp(r_lin));
    g = static_cast<uint8_t>(256 * intensity.clamp(g_lin));
    b = static_cast<uint8_t>(256 * intensity.clamp(b_lin));
}

inline void write_color(std::vector<uint8_t>& pixels, int index, const color& pixel) {
    uint8_t r, g, b;
    color_to_bytes(pixel, r, g, b);
    pixels[index + 0] = r;
    pixels[index + 1] = g;
    pixels[index + 2] = b;
}

#endif
