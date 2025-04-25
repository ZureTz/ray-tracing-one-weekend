#include "hittables/material.h"
#include "utils/interval.h"
#include "utils/ray.h"
#include "utils/rtweekend.h"
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

// Dielectric material

// Constructor, using refractive index
dielectric::dielectric(double refractive_index)
    : refractive_index(refractive_index) {}

// Schlick approximation for reflectance
double dielectric::reflectance(double cosine, double refraction_index) {
  auto r0 = (1 - refraction_index) / (1 + refraction_index);
  r0 = r0 * r0;
  return r0 + (1 - r0) * std::pow((1 - cosine), 5);
}

// Scatter function
bool dielectric::scatter(const ray &r_in, const hit_record &rec,
                         color &attenuation, ray &scattered) const {
  // Default attenuation color is white (or transparent)
  attenuation = color(1.0, 1.0, 1.0);

  // Calculate the refractive index ratio (n1/n2)
  const double ri =
      rec.front_face ? (1.0 / refractive_index) : refractive_index;
  // UV vector
  const vec3 unit_direction = unit_vector(r_in.direction());

  // Check if total internal reflection occurs
  const double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
  const double sin_theta_squared = 1.0 - cos_theta * cos_theta;

  // If ri * sin_theta > 1.0 (ri^2 * sin_theta^2 > 1.0),
  // total internal reflection occurs
  if ((ri * ri) * sin_theta_squared > 1.0 ||
      reflectance(cos_theta, ri) > random_double()) {
    // Reflect the ray
    const vec3 reflected_direction = reflect(unit_direction, rec.normal);
    scattered = ray(rec.point, reflected_direction);
    return true;
  }

  // Otherwise, calculate the refracted direction
  const vec3 refracted_direction = refract(unit_direction, rec.normal, ri);
  scattered = ray(rec.point, refracted_direction);
  return true;
}