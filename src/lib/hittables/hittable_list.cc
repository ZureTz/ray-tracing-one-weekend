#include <memory>

#include "../../include/hittables/hittable_list.h"

hittable_list::hittable_list(std::shared_ptr<hittable> object) { add(object); }

// Clear list
void hittable_list::clear() { objects.clear(); }
// Add an object to the list
void hittable_list::add(std::shared_ptr<hittable> object) {
  objects.push_back(object);
}

bool hittable_list::hit(const ray &r, double t_min, double t_max,
                        hit_record &record) const {
  // Find if any object is hit in the list
  // If found, return the nearest
  bool hit_anything = false;
  auto closest_t_so_far = t_max;

  for (const auto &object : objects) {
    // Create temporary hit_record
    hit_record temp_record;
    // If the object not hit, skip
    if (!object->hit(r, t_min, closest_t_so_far, temp_record)) {
      continue;
    }
    // Hit, update the closest t
    hit_anything = true;
    closest_t_so_far = temp_record.t;
    record = temp_record;
  }

  // Return true if any object is hit
  return hit_anything;
}