#pragma once
// Hittable objects are objects that can be hit by rays

#include "../utils/interval.h"
#include "../utils/ray.h"
#include "../utils/vec3.h"

struct hit_record {
  point3 point;
  vec3 normal;
  double t;
  bool front_face;

  hit_record();

  // Set front_face and normal based on the ray direction
  // NOTE: outward_normal is assumed to be a unit vector
  void set_face_normal(const ray &r, const vec3 &outward_normal);
};

class hittable {
public:
  virtual bool hit(const ray &r, interval ray_t, hit_record &record) const = 0;
  virtual ~hittable() = default;
};
