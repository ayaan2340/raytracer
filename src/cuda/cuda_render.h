#ifndef CUDA_RENDER_H
#define CUDA_RENDER_H

#include "camera.h"
#include "metrics.h"
#include "scene.h"

#include <cstdint>
#include <vector>

class CudaRenderer {
  public:
    explicit CudaRenderer(const SceneData& scene);
    ~CudaRenderer();

    CudaRenderer(const CudaRenderer&) = delete;
    CudaRenderer& operator=(const CudaRenderer&) = delete;

    bool available() const { return ok; }

    RenderMetrics render(RenderCamera& cam, std::vector<uint8_t>& pixels, int samples,
                         int prev_samples = 0, bool reset = true, uint32_t seed = 0);

  private:
    Scene d_scene{};
    color* d_accum = nullptr;
    uint8_t* d_fb = nullptr;
    int capacity = 0;
    bool ok = false;
};

#endif
