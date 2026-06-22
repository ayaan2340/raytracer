#ifndef INTERVAL_H
#define INTERVAL_H

#include "rtweekend.h"

class interval {
  public:
    Real min, max;

    RT_HOSTDEV interval() : min(+infinity), max(-infinity) {}
    RT_HOSTDEV interval(Real min, Real max) : min(min), max(max) {}

    RT_HOSTDEV Real size() const { return max - min; }

    RT_HOSTDEV bool contains(Real d) const { return min <= d && d <= max; }

    RT_HOSTDEV bool surrounds(Real d) const { return min < d && d < max; }

    RT_HOSTDEV Real clamp(Real d) const {
        if (d < min) return min;
        if (d > max) return max;
        return d;
    }

    static const interval empty, universe;
};

inline const interval interval::empty = interval();
inline const interval interval::universe = interval(-infinity, +infinity);

#endif
