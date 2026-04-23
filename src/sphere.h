#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable {
    public:
        sphere(const point3& center, double radius)
            : center{center}
            , radius{std::fmax(0.0, radius)} {}

        bool hit(const ray& r, double ray_tmin, double ray_tmax, hit_record& rec) const override {
            vec3 diff {center - r.origin()};
            auto a {r.direction().length_squared()};
            auto h {dot(r.direction(), diff)};
            auto c {diff.length_squared() - radius * radius};

            auto discriminant {h * h - a * c}; 
            if (discriminant < 0)
                return false;
            
            auto sqrtd = std::sqrt(discriminant);
            auto t {(h - sqrtd) / a};
            if (t <= ray_tmin || t >= ray_tmax) {
                t = (h + sqrtd) / a;
                if (t <= ray_tmin || t >= ray_tmax) {
                    return false;
                }
            }

            rec.set_face_normal(r, (r.at(t) - center) / radius);
            rec.p = r.at(t);
            rec.t = t;

            return true;
        }
        
    private:
        point3 center;
        double radius;

};

#endif