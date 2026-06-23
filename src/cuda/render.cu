#include "cuda_render.h"

#include "color.h"
#include "scene.h"

#include <cstdio>

#define CUDA_CHECK(call)                                                                       \
    do {                                                                                       \
        cudaError_t err = (call);                                                              \
        if (err != cudaSuccess) {                                                              \
            std::fprintf(stderr, "CUDA error %s:%d: %s\n", __FILE__, __LINE__,                 \
                         cudaGetErrorString(err));                                             \
        }                                                                                      \
    } while (0)

__global__ void render_kernel(color* accum, uint8_t* fb, int width, int height, CameraParams cam,
                              Scene scene, int samples, int max_depth, uint32_t seed,
                              int prev_samples, bool reset) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    if (i >= width || j >= height)
        return;

    int idx = j * width + i;
    rng rand(hash_combine(hash_combine(static_cast<uint32_t>(j), static_cast<uint32_t>(i)), seed));

    color sum(0, 0, 0);
    for (int s = 0; s < samples; ++s)
        sum += trace(cam.get_ray(i, j, rand), scene, rand, max_depth);

    color total = reset ? sum : accum[idx] + sum;
    accum[idx] = total;

    color averaged = total / static_cast<Real>(prev_samples + samples);
    uint8_t r, g, b;
    color_to_bytes(averaged, r, g, b);
    fb[idx * 3 + 0] = r;
    fb[idx * 3 + 1] = g;
    fb[idx * 3 + 2] = b;
}

CudaRenderer::CudaRenderer(const SceneData& scene) {
    int device_count = 0;
    if (cudaGetDeviceCount(&device_count) != cudaSuccess || device_count == 0) {
        std::fprintf(stderr, "No CUDA device available.\n");
        return;
    }

    Sphere* d_spheres = nullptr;
    SceneMaterial* d_materials = nullptr;
    size_t sphere_bytes = scene.spheres.size() * sizeof(Sphere);
    size_t material_bytes = scene.materials.size() * sizeof(SceneMaterial);

    CUDA_CHECK(cudaMalloc(&d_spheres, sphere_bytes));
    CUDA_CHECK(cudaMalloc(&d_materials, material_bytes));
    CUDA_CHECK(cudaMemcpy(d_spheres, scene.spheres.data(), sphere_bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_materials, scene.materials.data(), material_bytes, cudaMemcpyHostToDevice));

    d_scene.spheres = d_spheres;
    d_scene.sphere_count = static_cast<int>(scene.spheres.size());
    d_scene.materials = d_materials;
    d_scene.material_count = static_cast<int>(scene.materials.size());
    ok = true;
}

CudaRenderer::~CudaRenderer() {
    cudaFree(const_cast<Sphere*>(d_scene.spheres));
    cudaFree(const_cast<SceneMaterial*>(d_scene.materials));
    cudaFree(d_accum);
    cudaFree(d_fb);
}

RenderMetrics CudaRenderer::render(RenderCamera& cam, std::vector<uint8_t>& pixels, int samples,
                                   int prev_samples, bool reset, uint32_t seed) {
    const int width = cam.image_width;
    const int height = cam.image_height();
    const int pixel_count = width * height;
    cam.prepare();

    if (pixel_count > capacity) {
        cudaFree(d_accum);
        cudaFree(d_fb);
        CUDA_CHECK(cudaMalloc(&d_accum, pixel_count * sizeof(color)));
        CUDA_CHECK(cudaMalloc(&d_fb, pixel_count * 3 * sizeof(uint8_t)));
        capacity = pixel_count;
    }

    pixels.resize(static_cast<size_t>(pixel_count) * 3);

    Timer timer;
    dim3 block(16, 16);
    dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);
    render_kernel<<<grid, block>>>(d_accum, d_fb, width, height, cam.params, d_scene, samples,
                                   cam.max_depth, seed, prev_samples, reset);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    CUDA_CHECK(cudaMemcpy(pixels.data(), d_fb, pixels.size(), cudaMemcpyDeviceToHost));

    RenderMetrics metrics;
    metrics.width = width;
    metrics.height = height;
    metrics.samples_per_pixel = samples;
    metrics.max_depth = cam.max_depth;
    metrics.seconds = timer.elapsed_seconds();
    metrics.threads = pixel_count;
    return metrics;
}
