#ifndef SCENE_H
#define SCENE_H

#include "ray.h"
#include "vec3.h"

#include <vector>

enum class MatType { Lambertian, Metal, Dielectric };

struct SceneMaterial {
    MatType type;
    vec3 albedo;
    Real fuzz;
    Real ior;
};

struct Sphere {
    vec3 center;
    Real radius;
    int material_id;
};

struct HitRecord {
    vec3 p;
    vec3 normal;
    Real t;
    int material_id;
    bool front_face;
};

struct Scene {
    const Sphere* spheres;
    int sphere_count;
    const SceneMaterial* materials;
    int material_count;
};

struct SceneData {
    std::vector<SceneMaterial> materials;
    std::vector<Sphere> spheres;

    Scene view() const {
        return Scene{
            spheres.data(),
            static_cast<int>(spheres.size()),
            materials.data(),
            static_cast<int>(materials.size()),
        };
    }

    int add_material(const SceneMaterial& material) {
        materials.push_back(material);
        return static_cast<int>(materials.size()) - 1;
    }

    void add_sphere(const Sphere& sphere) { spheres.push_back(sphere); }
};

RT_HOSTDEV inline void set_face_normal(const ray& r, const vec3& outward_normal, HitRecord& rec) {
    rec.front_face = dot(r.direction(), outward_normal) < 0;
    rec.normal = rec.front_face ? outward_normal : -outward_normal;
}

RT_HOSTDEV inline bool hit_sphere(const Sphere& sphere, const ray& r, Real t_min, Real t_max,
                                  HitRecord& rec) {
    vec3 diff = sphere.center - r.origin();
    Real a = r.direction().length_squared();
    Real h = dot(r.direction(), diff);
    Real c = diff.length_squared() - sphere.radius * sphere.radius;
    Real discriminant = h * h - a * c;
    if (discriminant < 0)
        return false;

    Real sqrtd = std::sqrt(discriminant);
    Real t = (h - sqrtd) / a;
    if (t < t_min || t > t_max) {
        t = (h + sqrtd) / a;
        if (t < t_min || t > t_max)
            return false;
    }

    rec.t = t;
    rec.p = r.at(t);
    rec.material_id = sphere.material_id;
    set_face_normal(r, (rec.p - sphere.center) / sphere.radius, rec);
    return true;
}

RT_HOSTDEV inline bool hit_scene(const Scene& scene, const ray& r, Real t_min, Real t_max,
                                 HitRecord& rec) {
    HitRecord temp;
    bool hit_anything = false;
    Real closest = t_max;

    for (int i = 0; i < scene.sphere_count; ++i) {
        if (hit_sphere(scene.spheres[i], r, t_min, closest, temp)) {
            hit_anything = true;
            closest = temp.t;
            rec = temp;
        }
    }

    return hit_anything;
}

RT_HOSTDEV inline Real reflectance(Real cosine, Real refraction_index) {
    Real r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 = r0 * r0;
    return r0 + (1 - r0) * std::pow((1 - cosine), 5);
}

RT_HOSTDEV inline bool scatter(const SceneMaterial& mat, const ray& r_in, const HitRecord& rec,
                               vec3& attenuation, ray& scattered, rng& rand) {
    switch (mat.type) {
        case MatType::Lambertian: {
            vec3 direction = rec.normal + random_unit_vector(rand);
            if (direction.near_zero())
                direction = rec.normal;
            scattered = ray(rec.p, direction);
            attenuation = mat.albedo;
            return true;
        }
        case MatType::Metal: {
            vec3 reflected = reflect(r_in.direction(), rec.normal) + mat.fuzz * random_unit_vector(rand);
            scattered = ray(rec.p, reflected);
            attenuation = mat.albedo;
            return dot(scattered.direction(), rec.normal) > 0;
        }
        case MatType::Dielectric: {
            attenuation = vec3(1, 1, 1);
            Real ri = rec.front_face ? (1.0f / mat.ior) : mat.ior;
            vec3 unit_direction = unit_vector(r_in.direction());
            Real cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0f);
            Real sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);
            vec3 direction;
            if (ri * sin_theta > 1.0f || reflectance(cos_theta, ri) > rand.next_real())
                direction = reflect(unit_direction, rec.normal);
            else
                direction = refract(unit_direction, rec.normal, ri);
            scattered = ray(rec.p, direction);
            return true;
        }
    }
    return false;
}

RT_HOSTDEV inline vec3 sky_color(const ray& r) {
    Real a = 0.5f * (unit_vector(r.direction()).y() + 1.0f);
    return (1.0f - a) * vec3(1.0f, 1.0f, 1.0f) + a * vec3(0.4f, 0.35f, 1.0f);
}

RT_HOSTDEV inline vec3 trace(const ray& r, const Scene& scene, rng& rand, int max_depth) {
    vec3 throughput(1, 1, 1);
    ray current = r;

    for (int depth = 0; depth < max_depth; ++depth) {
        HitRecord rec;
        if (hit_scene(scene, current, 0.001f, infinity, rec)) {
            vec3 attenuation;
            ray scattered;
            const SceneMaterial& mat = scene.materials[rec.material_id];
            if (scatter(mat, current, rec, attenuation, scattered, rand)) {
                throughput = throughput * attenuation;
                current = scattered;
            } else {
                return vec3(0, 0, 0);
            }
        } else {
            return throughput * sky_color(current);
        }
    }

    return vec3(0, 0, 0);
}

SceneData build_final_scene();

#endif
