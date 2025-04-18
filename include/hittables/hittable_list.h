#pragma once
// A list of hittable objects (Partly world)

#include <memory>
#include <vector>

#include "hittables/hittable.h"

class hittable_list : public hittable {
public:
  // List of pointers to hittable objects
  std::vector<std::shared_ptr<hittable>> objects;

  hittable_list() = default;
  hittable_list(std::shared_ptr<hittable> object);

  // Clear list
  void clear();
  // Add an object to the list
  void add(std::shared_ptr<hittable> object);

  bool hit(const ray &r, interval ray_t, hit_record &record) const override;
};