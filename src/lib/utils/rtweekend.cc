#include <cstdlib>

#include "../../include/utils/rtweekend.h"

// Utility Functions implementations

double degrees_to_radians(double degrees) { return degrees * pi / 180.0; }

double random_double() {
  // Returns a random real in [0,1).
  return std::rand() / double(RAND_MAX + 1.0);
}

double random_double(double min, double max) {
  // Returns a random real in [min,max).
  return min + (max - min) * random_double();
}