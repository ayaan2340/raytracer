#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
  public:
    RT_HOSTDEV ray() = default;

    RT_HOSTDEV ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction) {}

    RT_HOSTDEV const point3& origin() const { return orig; }
    RT_HOSTDEV const vec3& direction() const { return dir; }
    RT_HOSTDEV point3 at(Real t) const { return orig + t * dir; }

  private:
    point3 orig;
    vec3 dir;
};

#endif
