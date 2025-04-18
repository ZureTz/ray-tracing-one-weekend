#include "hittables/material.h"
#include "utils/interval.h"
#include "utils/ray.h"
#include "utils/vec3.h"

// Set destructor to default
material::~material() = default;

// Set scatter to default
bool material::scatter(const ray &r_in, const hit_record &rec,
                       color &attenuation, ray &scattered) const {
  return false;
}

// Lambertian material

// Constructor, using color as albedo
lambertian::lambertian(const color &albedo) : albedo(albedo) {}

// Scatter function
bool lambertian::scatter(const ray &r_in, const hit_record &rec,
                         color &attenuation, ray &scattered) const {
  // Lambertian scatter
  auto scatter_direction = rec.normal + random_unit_vector();
  // Catch degenerate scatter direction
  if (scatter_direction.near_zero()) {
    // If the scatter direction is near zero, use the normal as the scatter
    // direction
    scatter_direction = rec.normal;
  }

  // Set the scatter direction
  scattered = ray(rec.point, scatter_direction);
  // Set the attenuation properties
  attenuation = albedo;

  return true;
}

// Metal material

// Constructor, using color as albedo, and fuzziness
metal::metal(const color &albedo, const double fuzz)
    : albedo(albedo), fuzz(interval(0.0, 1.0).clamp(fuzz)) {}

// Scatter function
bool metal::scatter(const ray &r_in, const hit_record &rec, color &attenuation,
                    ray &scattered) const {
  // Metal scatter
  vec3 reflected_direction = reflect(r_in.direction(), rec.normal);
  // Add fuzziness to the reflected direction
  reflected_direction =
      unit_vector(reflected_direction) + (fuzz * random_unit_vector());
  scattered = ray(rec.point, reflected_direction);
  attenuation = albedo;
  // Check if the scattered ray is in the same hemisphere as the normal
  return (dot(scattered.direction(), rec.normal) > 0);
}