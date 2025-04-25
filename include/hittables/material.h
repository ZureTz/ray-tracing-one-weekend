#pragma once

#include "hittables/hittable.h"
#include "utils/color.h"

// Abstract base class for materials
class material {
public:
  virtual bool scatter(const ray &r_in, const hit_record &rec,
                       color &attenuation, ray &scattered) const;

  virtual ~material();
};

// Lambertian material
class lambertian : public material {
  color albedo;

public:
  // Constructor, using color as albedo
  lambertian(const color &albedo);

  // Scatter function
  bool scatter(const ray &r_in, const hit_record &rec, color &attenuation,
               ray &scattered) const override;
};

// Metal material
class metal : public material {
  color albedo;
  double fuzz;

public:
  // Constructor, using color as albedo, and fuzziness
  metal(const color &albedo, const double fuzz);

  // Scatter function
  bool scatter(const ray &r_in, const hit_record &rec, color &attenuation,
               ray &scattered) const override;
};

// Dielectric material
class dielectric : public material {
  // Refractive index in vacuum or air, or the ratio of the material's
  // refractive index over the refractive index of the enclosing media
  double refractive_index;

  // Schlick approximation for reflectance
  static double reflectance(double cosine, double refraction_index);

public:
  // Constructor, using refractive index
  dielectric(double refractive_index);

  // Scatter function
  bool scatter(const ray &r_in, const hit_record &rec, color &attenuation,
               ray &scattered) const override;
};
