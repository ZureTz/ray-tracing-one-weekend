#include "../../utils/ray.h"

// Constructors
ray::ray() {}
ray::ray(const point3 &origin, const vec3 &direction)
    : orig(origin), dir(direction) {}

// Gets
const point3 &ray::origin() const { return orig; }
const vec3 &ray::direction() const { return dir; }

// At
point3 ray::at(double t) const { return orig + t * dir; }