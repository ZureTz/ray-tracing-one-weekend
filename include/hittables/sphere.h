#pragma once
// Sphere class derived from hittable

#include "hittables/hittable.h"

class sphere : public hittable {
  point3 center;
  double radius;

public:
  sphere(const point3 &center, const double radius);

  // Determine if the ray hits the sphere
  bool hit(const ray &r, interval ray_t, hit_record &record) const override;
  ~sphere() override = default;
};