#include "color.h"
#include "vec3.h"
#include "ray.h"
#include <iostream>
#include <algorithm>

color ray_color(const ray& r) {
    return color(0, 0.5 * (unit_vector(r.direction()).y() + 1.0), 0);
}


int main() {

    // Image
    constexpr auto aspect_ratio {16.0 / 9.0};
    constexpr int image_width {400};
    constexpr int image_height {std::max(1, static_cast<int>(image_width / aspect_ratio))};

    // Camera
    constexpr double viewport_height {2.0};
    constexpr double viewport_width {viewport_height * (double(image_width) / image_height)};
    constexpr double focal_length {1.0};
    constexpr point3 camera_center {0, 0, 0};

    // Horizontal (u) and vertical (v) viewport vectors
    constexpr vec3 viewport_u {viewport_width, 0, 0};
    constexpr vec3 viewport_v {0, -viewport_height, 0};

    // Horizontal (u) and vertical (v) pixel delta vectors
    constexpr vec3 pixel_delta_u {viewport_u / image_width};
    constexpr vec3 pixel_delta_v {viewport_v / image_height};

    // Location of upper left pixel
    constexpr point3 viewport_upper_left {camera_center 
                                         - vec3(0, 0, focal_length) 
                                         - viewport_u / 2 - viewport_v / 2};
    constexpr point3 pixel00_location {viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v)};

    // Render
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            auto pixel_center {pixel00_location + (i * pixel_delta_u) + (j * pixel_delta_v)};
            auto ray_direction {pixel_center - camera_center};
            ray r {camera_center, ray_direction};

            color pixel_color {ray_color(r)};
            write_color(std::cout, pixel_color);
        }
    }
    std::clog << "\rDone.                 \n";
    return 0;
}