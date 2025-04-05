#include "../../include/hittables/hittable.h"
#include "../../include/utils/vec3.h"

hit_record::hit_record() : point(), normal(), t(0) {}
// Set front_face and normal based on the ray direction
// NOTE: outward_normal is assumed to be a unit vector
void hit_record::set_face_normal(const ray &r, const vec3 &outward_normal) {
  // Dot product of the ray direction and the outward normal
  // positive: negate the normal
  // negative: keep the normal
  // If the dot product is negative, the ray is outside the object, set
  // front_face to true
  front_face = dot(r.direction(), outward_normal) < 0;
  normal = front_face ? outward_normal : -outward_normal;
}