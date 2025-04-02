#pragma once
// Hittable objects are objects that can be hit by rays

#include "ray.h"
#include "vec3.h"

struct hit_record {
  point3 point;
  vec3 normal;
  double t;

  hit_record() ;
};

class hittable {
public:
  virtual bool hit(const ray &r, double t_min, double t_max,
                   hit_record &record) const = 0;
  virtual ~hittable() = default;
};
