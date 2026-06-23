#include "camera.h"
#include "scene.h"

#ifdef USE_CUDA
#include "cuda/cuda_render.h"
#endif

#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    bool benchmark = false;
    bool use_cuda = false;
    std::string output_path = "images/final_scene.png";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--benchmark")
            benchmark = true;
        else if (arg == "--cuda")
            use_cuda = true;
        else if (arg == "--output" && i + 1 < argc)
            output_path = argv[++i];
    }

    SceneData scene_data = build_final_scene();
    Scene world = scene_data.view();

    RenderCamera cam;
    cam.aspect_ratio = 16.0f / 9.0f;
    cam.image_width = 1200;
    cam.samples_per_pixel = 500;
    cam.max_depth = 50;
    cam.vfov = 20;
    cam.lookfrom = point3(13, 2, 3);
    cam.lookat = point3(0, 0, 0);
    cam.vup = vec3(0, 1, 0);
    cam.defocus_angle = 0.6f;
    cam.focus_dist = 10.0f;

    std::vector<uint8_t> pixels;

    if (benchmark) {
        cam.image_width = 400;
        cam.samples_per_pixel = 50;
        cam.use_openmp = false;
        RenderMetrics single_thread = cam.render(world, pixels);
        single_thread.print("Single-threaded CPU");

        cam.use_openmp = true;
        RenderMetrics multi_thread = cam.render(world, pixels);
        multi_thread.print("OpenMP CPU");

        if (single_thread.seconds > 0) {
            std::printf("\nSpeedup: %.2fx\n", single_thread.seconds / multi_thread.seconds);
        }

#ifdef USE_CUDA
        CudaRenderer cuda(scene_data);
        if (cuda.available()) {
            RenderMetrics gpu = cuda.render(cam, pixels, cam.samples_per_pixel);
            gpu.print("CUDA GPU");
            if (gpu.seconds > 0)
                std::printf("\nCUDA speedup vs OpenMP: %.2fx\n", multi_thread.seconds / gpu.seconds);
        }
#endif
    } else if (use_cuda) {
#ifdef USE_CUDA
        CudaRenderer cuda(scene_data);
        if (!cuda.available()) {
            std::fprintf(stderr, "CUDA unavailable, aborting.\n");
            return 1;
        }
        RenderMetrics metrics = cuda.render(cam, pixels, cam.samples_per_pixel);
        metrics.print("CUDA render complete");
#else
        std::fprintf(stderr, "Built without CUDA support; rebuild with the CUDA toolkit.\n");
        return 1;
#endif
    } else {
        cam.use_openmp = true;
        RenderMetrics metrics = cam.render(world, pixels);
        metrics.print("Render complete");
    }

    if (!cam.save_png(pixels, output_path.c_str())) {
        std::fprintf(stderr, "Failed to write %s\n", output_path.c_str());
        return 1;
    }

    std::printf("Wrote %s\n", output_path.c_str());
    return 0;
}
