#include "../../include/hittables/sphere.h"

sphere::sphere(const point3 &center, const double radius)
    : center(center), radius(std::max(0.0, radius)) {}

// Determine if the ray hits the sphere
bool sphere::hit(const ray &r, interval ray_t, hit_record &record) const {
  // t^2⋅d⋅d−2t⋅d⋅(C−Q)+(C−Q)⋅(C−Q)−r^2=0
  // a = d⋅d
  // b = -2⋅d⋅(C−Q); h = d⋅(C−Q)
  // c = (C−Q)⋅(C−Q)−r^2

  const auto d = r.direction();
  const auto C_Q = center - r.origin();

  const auto a = d.length_squared();
  const auto h = dot(d, C_Q);
  const auto c = C_Q.length_squared() - radius * radius;

  // Determine if it has root: b^2 - 4ac >= 0
  const auto discriminant = h * h - a * c;

  // If not hit
  if (discriminant < 0) {
    return false;
  }

  // If hit, first calculate root and check if it's in range
  const auto sqrt_discriminant = sqrt(discriminant);
  // nearer root
  auto root = (h - sqrt_discriminant) / a;
  // Check if at least one root is in range
  if (!ray_t.surrounds(root)) {
    // If one root missed, change root to another
    root = (h + sqrt_discriminant) / a;
    // Check another root
    if (!ray_t.surrounds(root)) {
      return false;
    }
    // Otherwise we have a hit
  }

  // Then fill the hit record
  record.t = root;
  record.point = r.at(record.t);
  const vec3 outward_normal = (record.point - center) / radius;
  record.set_face_normal(r, outward_normal);

  return true;
}