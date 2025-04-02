#pragma once
// Sphere class derived from hittable

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable {
  point3 center;
  double radius;

public:
  sphere(const point3 &center, const double radius);

  // Determine if the ray hits the sphere
  bool hit(const ray &r, double t_min, double t_max,
           hit_record &record) const override;
  ~sphere() override = default;
};