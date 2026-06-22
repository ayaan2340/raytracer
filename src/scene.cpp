#include "scene.h"

#include "rtweekend.h"

SceneData build_final_scene() {
    SceneData scene;

    int ground_mat = scene.add_material({MatType::Lambertian, vec3(0.45f, 0.42f, 0.38f), 0, 1.5f});
    scene.add_sphere({vec3(0, -1000, 0), 1000, ground_mat});

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            rng choose_rng(hash_combine(static_cast<uint32_t>(a), static_cast<uint32_t>(b)));
            Real choose_mat = choose_rng.next_real();
            point3 center(a + 0.9f * choose_rng.next_real(), 0.2f, b + 0.9f * choose_rng.next_real());

            if ((center - point3(0, 1, 0)).length() > 1.3f &&
                (center - point3(-4, 1, 0)).length() > 1.3f &&
                (center - point3(4, 1, 0)).length() > 1.3f) {
                if (choose_mat < 0.60f) {
                    rng albedo_rng(hash_combine(choose_rng.next_u32(), 1u));
                    vec3 albedo = vec3(albedo_rng.next_real(), albedo_rng.next_real(), albedo_rng.next_real()) *
                                  vec3(albedo_rng.next_real(), albedo_rng.next_real(), albedo_rng.next_real());
                    int mat_id = scene.add_material({MatType::Lambertian, albedo, 0, 1.5f});
                    scene.add_sphere({center, 0.2f, mat_id});
                } else if (choose_mat < 0.85f) {
                    rng albedo_rng(hash_combine(choose_rng.next_u32(), 2u));
                    vec3 albedo(albedo_rng.next_real(0.5f, 1), albedo_rng.next_real(0.5f, 1),
                                albedo_rng.next_real(0.5f, 1));
                    Real fuzz = choose_rng.next_real(0, 0.4f);
                    int mat_id = scene.add_material({MatType::Metal, albedo, fuzz, 1.5f});
                    scene.add_sphere({center, 0.2f, mat_id});
                } else {
                    int mat_id = scene.add_material({MatType::Dielectric, vec3(1, 1, 1), 0, 1.5f});
                    scene.add_sphere({center, 0.2f, mat_id});
                }
            }
        }
    }

    int glass = scene.add_material({MatType::Dielectric, vec3(1, 1, 1), 0, 1.5f});
    scene.add_sphere({vec3(0, 1, 0), 1.0f, glass});
    scene.add_sphere({vec3(0, 1, 0), -0.87f, scene.add_material({MatType::Dielectric, vec3(1, 1, 1), 0, 1.0f / 1.5f})});

    int left_mat = scene.add_material({MatType::Lambertian, vec3(0.15f, 0.10f, 0.55f), 0, 1.5f});
    scene.add_sphere({vec3(-4, 1, 0), 1.0f, left_mat});

    int center_mat = scene.add_material({MatType::Metal, vec3(0.85f, 0.65f, 0.15f), 0.02f, 1.5f});
    scene.add_sphere({vec3(4, 1, 0), 1.0f, center_mat});

    int accent_mat = scene.add_material({MatType::Metal, vec3(0.72f, 0.40f, 0.20f), 0.25f, 1.5f});
    scene.add_sphere({vec3(4.55f, 2.35f, -0.55f), 0.35f, accent_mat});

    int bubble_mat = scene.add_material({MatType::Dielectric, vec3(1, 1, 1), 0, 1.5f});
    scene.add_sphere({vec3(-4.6f, 0.35f, 1.1f), 0.35f, bubble_mat});

    return scene;
}
