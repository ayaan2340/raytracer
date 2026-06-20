#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

int main() {

    hittable_list world;

    auto ground_mat = make_shared<lambertian>(color(0.45, 0.42, 0.38));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_mat));

    // Random spheres placed around the ground
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            double choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(0, 1, 0)).length()  > 1.3 &&
                (center - point3(-4, 1, 0)).length() > 1.3 &&
                (center - point3(4, 1, 0)).length()  > 1.3)
            {
                if (choose_mat < 0.60) {
                    auto albedo = color(random_double(), random_double(), random_double())
                                * color(random_double(), random_double(), random_double());
                    world.add(make_shared<sphere>(center, 0.2,
                        make_shared<lambertian>(albedo)));

                } else if (choose_mat < 0.85) {
                    auto albedo = color(random_double(0.5, 1), random_double(0.5, 1), random_double(0.5, 1));
                    auto fuzz   = random_double(0, 0.4);
                    world.add(make_shared<sphere>(center, 0.2,
                        make_shared<metal>(albedo, fuzz)));

                } else {
                    world.add(make_shared<sphere>(center, 0.2,
                        make_shared<dielectric>(1.5)));
                }
            }
        }
    }

    // Glass sphere placed in the center
    auto glass = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3( 0, 1, 0),  1.0, glass));
    world.add(make_shared<sphere>(point3( 0, 1, 0), -0.87, make_shared<dielectric>(1.0 / 1.5)));

    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, make_shared<lambertian>(color(0.15, 0.10, 0.55))));
    world.add(make_shared<sphere>(point3( 4, 1, 0), 1.0, make_shared<metal>(color(0.85, 0.65, 0.15), 0.02)));
    world.add(make_shared<sphere>(point3(4.55, 2.35, -0.55), 0.35, make_shared<metal>(color(0.72, 0.40, 0.20), 0.25)));
    world.add(make_shared<sphere>(point3(-4.6, 0.35, 1.1), 0.35, make_shared<dielectric>(1.5)));

    // Camera 
    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13, 2, 3);
    cam.lookat   = point3(0, 0, 0);
    cam.vup      = vec3(0, 1, 0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world);

    return 0;
}