#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "metrics.h"
#include "scene.h"

#include <algorithm>
#include <cstdio>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

struct CameraParams {
    point3 center;
    point3 pixel00_loc;
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;
    vec3 defocus_disk_u;
    vec3 defocus_disk_v;
    Real defocus_angle = 0;

    RT_HOSTDEV ray get_ray(int i, int j, rng& rand) const {
        vec3 offset(rand.next_real() - 0.5f, rand.next_real() - 0.5f, 0);
        point3 pixel_sample = pixel00_loc + (static_cast<Real>(i) + offset.x()) * pixel_delta_u +
                              (static_cast<Real>(j) + offset.y()) * pixel_delta_v;
        point3 ray_origin = center;
        if (defocus_angle > 0) {
            vec3 p = random_in_unit_disk(rand);
            ray_origin = center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }
        return ray(ray_origin, pixel_sample - ray_origin);
    }
};

struct RenderCamera {
    Real aspect_ratio = 1.0f;
    int image_width = 100;
    int samples_per_pixel = 10;
    int max_depth = 10;
    Real vfov = 90.0f;
    point3 lookfrom = point3(0, 0, 0);
    point3 lookat = point3(0, 0, -1);
    vec3 vup = vec3(0, 1, 0);
    Real defocus_angle = 0;
    Real focus_dist = 10;
    bool use_openmp = true;

    int image_height() const { return std::max(1, static_cast<int>(image_width / aspect_ratio)); }

    void prepare() { initialize(image_height()); }

    RenderMetrics render(const Scene& world, std::vector<uint8_t>& pixels) {
        const int height = image_height();
        prepare();
        pixels.resize(static_cast<size_t>(image_width) * height * 3);

        Timer timer;
        const Real sample_scale = 1.0f / samples_per_pixel;

#ifdef _OPENMP
        const int thread_count = use_openmp ? omp_get_max_threads() : 1;
#else
        const int thread_count = 1;
#endif

        if (use_openmp) {
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
            for (int j = 0; j < height; j++) {
                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0, 0, 0);
                    rng pixel_rng(hash_combine(static_cast<uint32_t>(j), static_cast<uint32_t>(i)));

                    for (int sample = 0; sample < samples_per_pixel; sample++) {
                        ray r = get_ray(i, j, pixel_rng);
                        pixel_color += trace(r, world, pixel_rng, max_depth);
                    }

                    const int index = (j * image_width + i) * 3;
                    write_color(pixels, index, sample_scale * pixel_color);
                }
            }
        } else {
            for (int j = 0; j < height; j++) {
                std::fprintf(stderr, "\rScanlines remaining: %d ", height - j);
                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0, 0, 0);
                    rng pixel_rng(hash_combine(static_cast<uint32_t>(j), static_cast<uint32_t>(i)));

                    for (int sample = 0; sample < samples_per_pixel; sample++) {
                        ray r = get_ray(i, j, pixel_rng);
                        pixel_color += trace(r, world, pixel_rng, max_depth);
                    }

                    const int index = (j * image_width + i) * 3;
                    write_color(pixels, index, sample_scale * pixel_color);
                }
            }
            std::fprintf(stderr, "\rDone.                 \n");
        }

        RenderMetrics metrics;
        metrics.width = image_width;
        metrics.height = height;
        metrics.samples_per_pixel = samples_per_pixel;
        metrics.max_depth = max_depth;
        metrics.seconds = timer.elapsed_seconds();
        metrics.threads = thread_count;
        return metrics;
    }

    ray get_ray(int i, int j, rng& rand) { return params.get_ray(i, j, rand); }

    CameraParams params;

  private:
    void initialize(int height) {
        Real theta = degrees_to_radians(vfov);
        Real h = std::tan(theta / 2);
        Real viewport_height = h * 2 * focus_dist;
        Real viewport_width = viewport_height * (static_cast<Real>(image_width) / height);

        vec3 w = unit_vector(lookfrom - lookat);
        vec3 u = unit_vector(cross(vup, w));
        vec3 v = cross(w, u);

        vec3 viewport_u = viewport_width * u;
        vec3 viewport_v = viewport_height * -v;

        params.center = lookfrom;
        params.pixel_delta_u = viewport_u / image_width;
        params.pixel_delta_v = viewport_v / height;

        point3 viewport_upper_left = lookfrom - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        params.pixel00_loc = viewport_upper_left + 0.5f * (params.pixel_delta_u + params.pixel_delta_v);

        Real defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        params.defocus_disk_u = u * defocus_radius;
        params.defocus_disk_v = v * defocus_radius;
        params.defocus_angle = defocus_angle;
    }

  public:
    bool save_png(const std::vector<uint8_t>& pixels, const char* path);
};

#endif
