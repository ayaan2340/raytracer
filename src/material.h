#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

class material {
    public:
        virtual ~material() = default;

        virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const {
            return false;
        }
};

class lambertian : public material {
    public:
        lambertian(const color& albedo) : albedo {albedo} {};

        bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
            auto direction {rec.normal + random_unit_vector()};
            if (direction.near_zero())
                direction = rec.normal;
            scattered = ray(rec.p, direction);

            attenuation = albedo;
            return true;
        }

    private:
        color albedo;
};

class metal : public material {
    public:
        metal(const color& albedo, double fuzz) : albedo {albedo}, fuzz {fuzz < 1 ? fuzz : 1} {}

        bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
            vec3 reflected {reflect(r_in.direction(), rec.normal) + (fuzz * random_unit_vector())};
            scattered = ray(rec.p, reflected);
            attenuation = albedo;
            return (dot(scattered.direction(), rec.normal) > 0);
        }    
    
    private:
        color albedo;
        double fuzz;

};

#endif