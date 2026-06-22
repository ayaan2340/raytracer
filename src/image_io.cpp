#include "camera.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"

bool RenderCamera::save_png(const std::vector<uint8_t>& pixels, const char* path) {
    return stbi_write_png(path, image_width, image_height(), 3, pixels.data(), image_width * 3) != 0;
}
