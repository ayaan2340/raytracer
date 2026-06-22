#include "scene.h"

#include "raylib.h"

#include "camera.h"

#include <cmath>
#include <cstdio>
#include <vector>

struct FlyCamera {
    point3 position;
    point3 target;
    vec3 up = vec3(0, 1, 0);
    Real yaw = 0;
    Real pitch = 0;
    Real move_speed = 5.0f;
    Real look_sensitivity = 0.0015f;
    Real vfov = 20;
    Real defocus_angle = 0.6f;
    Real focus_dist = 10.0f;

    void sync_angles_from_target() {
        vec3 forward = unit_vector(target - position);
        pitch = std::asin(forward.y());
        yaw = std::atan2(forward.x(), forward.z());
    }

    void update_from_input(float dt, bool& moved) {
        moved = false;
        vec3 forward_flat(std::sin(yaw), 0, std::cos(yaw));
        vec3 right = unit_vector(cross(forward_flat, up));

        if (IsKeyDown(KEY_W)) {
            position += forward_flat * move_speed * dt;
            moved = true;
        }
        if (IsKeyDown(KEY_S)) {
            position -= forward_flat * move_speed * dt;
            moved = true;
        }
        if (IsKeyDown(KEY_A)) {
            position -= right * move_speed * dt;
            moved = true;
        }
        if (IsKeyDown(KEY_D)) {
            position += right * move_speed * dt;
            moved = true;
        }
        if (IsKeyDown(KEY_SPACE)) {
            position += up * move_speed * dt;
            moved = true;
        }
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            position -= up * move_speed * dt;
            moved = true;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            yaw -= delta.x * look_sensitivity;
            pitch -= delta.y * look_sensitivity;
            pitch = std::clamp(pitch, -1.4f, 1.4f);
            moved = true;
        }

        vec3 forward(std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch));
        target = position + forward;
    }

    void apply_to(RenderCamera& cam) const {
        cam.lookfrom = position;
        cam.lookat = target;
        cam.vup = up;
        cam.vfov = vfov;
        cam.defocus_angle = defocus_angle;
        cam.focus_dist = focus_dist;
    }
};

int main() {
    const int render_width = 960;
    const int render_height = 540;
    const int samples_per_frame = 1;
    const int max_depth = 20;

    SceneData scene_data = build_final_scene();
    Scene world = scene_data.view();

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(render_width, render_height, "GPU-Ready Path Tracer Viewer");
    SetTargetFPS(60);

    RenderCamera cam;
    cam.aspect_ratio = static_cast<Real>(render_width) / render_height;
    cam.image_width = render_width;
    cam.samples_per_pixel = samples_per_frame;
    cam.max_depth = max_depth;
    cam.use_openmp = true;

    FlyCamera fly;
    fly.position = point3(13, 2, 3);
    fly.target = point3(0, 0, 0);
    fly.sync_angles_from_target();

    std::vector<color> accumulation(render_width * render_height, color(0, 0, 0));
    std::vector<uint8_t> pixels(render_width * render_height * 3);
    int accumulated_samples = 0;
    bool camera_moved = true;

    Image image = GenImageColor(render_width, render_height, BLACK);
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        bool moved = false;
        fly.update_from_input(dt, moved);

        if (moved) {
            camera_moved = true;
            accumulated_samples = 0;
            std::fill(accumulation.begin(), accumulation.end(), color(0, 0, 0));
        }

        fly.apply_to(cam);
        cam.prepare();

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
        for (int j = 0; j < render_height; ++j) {
            for (int i = 0; i < render_width; ++i) {
                rng pixel_rng(hash_combine(static_cast<uint32_t>(j), static_cast<uint32_t>(i)));
                pixel_rng.state = hash_combine(pixel_rng.state, static_cast<uint32_t>(accumulated_samples));

                color sample_color(0, 0, 0);
                for (int s = 0; s < samples_per_frame; ++s) {
                    ray r = cam.get_ray(i, j, pixel_rng);
                    sample_color += trace(r, world, pixel_rng, max_depth);
                }

                const int idx = j * render_width + i;
                accumulation[idx] += sample_color / static_cast<Real>(samples_per_frame);
            }
        }

        accumulated_samples += samples_per_frame;
        const Real inv_samples = 1.0f / accumulated_samples;

        for (int j = 0; j < render_height; ++j) {
            for (int i = 0; i < render_width; ++i) {
                const int idx = j * render_width + i;
                const int px = idx * 3;
                write_color(pixels, px, accumulation[idx] * inv_samples);
            }
        }

        UpdateTexture(texture, pixels.data());

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(texture, 0, 0, WHITE);

        DrawText("WASD move | Right mouse look | Shift/Space up/down", 12, 12, 18, LIGHTGRAY);
        DrawText(TextFormat("FPS: %d", GetFPS()), 12, 36, 20, RAYWHITE);
        DrawText(TextFormat("Accumulated samples: %d", accumulated_samples), 12, 62, 20, RAYWHITE);
        DrawText(TextFormat("Resolution: %dx%d", render_width, render_height), 12, 88, 20, RAYWHITE);
        if (camera_moved && accumulated_samples < 8)
            DrawText("Move camera to explore. Image refines when you stop.", 12, 114, 18, YELLOW);

        EndDrawing();
        camera_moved = moved;
    }

    UnloadTexture(texture);
    CloseWindow();
    return 0;
}
