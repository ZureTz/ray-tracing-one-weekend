#pragma once

#include <array>
#include <ostream>

#include <toml++/toml.hpp>

class vec3 {
  // A array with fixed length 3
  std::array<double, 3> e;

public:
  // Initializers
  vec3();
  vec3(double e0, double e1, double e2);
  vec3(const toml::array &arr);

  // Get
  double x() const;
  double y() const;
  double z() const;

  // Negate
  vec3 operator-() const;
  // Get specific coordinate using index (const)
  double operator[](int i) const;
  // Get specific coordinate using index (non-const, reference)
  double &operator[](int i);

  // Add with others
  vec3 operator+(const vec3 &v) const;

  // Subtract with others
  vec3 operator-(const vec3 &v) const;

  // Multiply with others
  vec3 operator*(const vec3 &v) const;
  // Add together, write to self
  vec3 &operator+=(const vec3 &v);

  // Subtract together, write to self
  vec3 &operator-=(const vec3 &v);

  // Multiply together, write to self
  vec3 &operator*=(const vec3 &v);

  // Multiply by a scalar (*=)
  vec3 &operator*=(const double t);

  // Divide by a scalar (/=)
  vec3 &operator/=(const double t);

  // Length in square
  double length_squared() const;

  // Length
  double length() const;

  // Random vec3
  static vec3 random();
  static vec3 random(double min, double max);
};

// point3 is just an alias for vec3, but useful for geometric clarity in the
// code.
using point3 = vec3;

// Vector Utility Functions

// Output vec3
std::ostream &operator<<(std::ostream &out, const vec3 &v);

// Multiply with a scalar
vec3 operator*(double t, const vec3 &v);
// Multiply with a scalar
vec3 operator*(const vec3 &v, double t);
// Divide by a scalar
vec3 operator/(const vec3 &v, double t);

// Dot product
double dot(const vec3 &u, const vec3 &v);
// Cross product
vec3 cross(const vec3 &u, const vec3 &v);

// Unit vector
vec3 unit_vector(vec3 v);

// Generate random unit vector
vec3 random_unit_vector();

// Generate unit vector is in the correct hemisphere based on the normal vector
vec3 random_in_hemisphere(const vec3 &normal);
