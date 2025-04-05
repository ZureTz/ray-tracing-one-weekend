#pragma once

#include "vec3.h"

class ray {
  point3 orig;
  vec3 dir;

public:
  // Constructors
  ray();
  ray(const point3 &origin, const vec3 &direction);

  // Gets
  const point3 &origin() const;
  const vec3 &direction() const;

  // At
  point3 at(double t) const;
};