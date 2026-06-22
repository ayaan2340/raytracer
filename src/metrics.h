#ifndef METRICS_H
#define METRICS_H

#include <chrono>
#include <cstdio>
#include <string>

struct Timer {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    double elapsed_seconds() const {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }

    void reset() { start = std::chrono::steady_clock::now(); }
};

struct RenderMetrics {
    int width;
    int height;
    int samples_per_pixel;
    int max_depth;
    double seconds;
    int threads;

    double total_rays() const {
        return static_cast<double>(width) * height * samples_per_pixel;
    }

    double mrays_per_second() const {
        if (seconds <= 0)
            return 0;
        return total_rays() / seconds / 1'000'000.0;
    }

    void print(const char* label) const {
        std::printf("%s\n", label);
        std::printf("  Resolution:     %d x %d\n", width, height);
        std::printf("  Samples/pixel:  %d\n", samples_per_pixel);
        std::printf("  Max depth:      %d\n", max_depth);
        std::printf("  Threads:        %d\n", threads);
        std::printf("  Render time:    %.2f s\n", seconds);
        std::printf("  Throughput:     %.2f Mrays/s\n", mrays_per_second());
    }
};

#endif
