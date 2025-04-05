#include "../../include/utils/interval.h"
#include "../../include/utils/rtweekend.h"

// Default empty interval
interval::interval() : min(+infinity), max(-infinity) {}
interval::interval(double min, double max) : min(min), max(max) {}

// Size of the interval
double interval::size() const { return max - min; }

// Contains and surrounds
bool interval::contains(double x) const { return min <= x && x <= max; }
bool interval::surrounds(double x) const { return min < x && x < max; }

// Two static special intervals
const interval interval::empty = interval(infinity, -infinity);
const interval interval::universe = interval(-infinity, infinity);
