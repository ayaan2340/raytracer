#ifndef INTERVAL_H
#define INTERVAL_H

class interval {
    public:
        double min, max;

        interval() : min {+infinity}, max {-infinity} {} // Default is empty
        interval(double min, double max) : min {min}, max {max} {}

        double size() const {
            return max - min;
        }

        bool contains(double d) const {
            return min <= d && d <= max;
        }

        bool surrounds(double d) const {
            return min < d && d < max;
        }

        static const interval empty, universe;
};

const interval interval::empty = interval();
const interval interval::universe = interval(-infinity, +infinity);

#endif