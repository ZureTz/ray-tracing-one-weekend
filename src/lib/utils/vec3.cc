#include <cmath>

#include "utils/vec3.h"
#include "utils/rtweekend.h"

// Initializers
vec3::vec3() : e{0, 0, 0} {}
vec3::vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}
vec3::vec3(const toml::array &arr) {
  if (arr.size() != 3) {
    throw std::runtime_error(
        "vec3 constructor requires a toml::array of size 3");
  }
  e[0] = arr[0].as_floating_point()->get();
  e[1] = arr[1].as_floating_point()->get();
  e[2] = arr[2].as_floating_point()->get();
}

// Get
double vec3::x() const { return e[0]; }
double vec3::y() const { return e[1]; }
double vec3::z() const { return e[2]; }

// Negate
vec3 vec3::operator-() const { return vec3(-e[0], -e[1], -e[2]); }
// Get specific coordinate using index (const)
double vec3::operator[](int i) const { return e[i]; }
// Get specific coordinate using index (non-const, reference)
double &vec3::operator[](int i) { return e[i]; }

// Add with others
vec3 vec3::operator+(const vec3 &v) const {
  return vec3(e[0] + v.e[0], e[1] + v.e[1], e[2] + v.e[2]);
}

// Subtract with others
vec3 vec3::operator-(const vec3 &v) const {
  return vec3(e[0] - v.e[0], e[1] - v.e[1], e[2] - v.e[2]);
}

// Multiply with others
vec3 vec3::operator*(const vec3 &v) const {
  return vec3(e[0] * v.e[0], e[1] * v.e[1], e[2] * v.e[2]);
}

// Add together, write to self
vec3 &vec3::operator+=(const vec3 &v) {
  e[0] += v.e[0];
  e[1] += v.e[1];
  e[2] += v.e[2];
  return *this;
}

// Subtract together, write to self
vec3 &vec3::operator-=(const vec3 &v) {
  e[0] -= v.e[0];
  e[1] -= v.e[1];
  e[2] -= v.e[2];
  return *this;
}

// Multiply together, write to self
vec3 &vec3::operator*=(const vec3 &v) {
  e[0] *= v.e[0];
  e[1] *= v.e[1];
  e[2] *= v.e[2];
  return *this;
}

// Multiply by a scalar (*=)
vec3 &vec3::operator*=(const double t) {
  e[0] *= t;
  e[1] *= t;
  e[2] *= t;
  return *this;
}

// Divide by a scalar (/=)
vec3 &vec3::operator/=(const double t) {
  if (std::fabs(t) < 1e-8) {
    throw std::runtime_error("Division by zero in vec3");
  }
  return *this *= 1 / t;
}

// Length in square
double vec3::length_squared() const {
  return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
}

// Length
double vec3::length() const { return std::sqrt(length_squared()); }

// Random vec3
vec3 vec3::random() {
  return vec3(random_double(), random_double(), random_double());
}

vec3 vec3::random(double min, double max) {
  return vec3(random_double(min, max), random_double(min, max),
              random_double(min, max));
}

// Return true if the vector is close to zero in all dimensions
bool vec3::near_zero() const {
  // Note that we compare the component-wise with a small value
  const double s = 1e-8;
  return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) &&
         (std::fabs(e[2]) < s);
}

// Vector Utility Functions

// Output vec3
std::ostream &operator<<(std::ostream &out, const vec3 &v) {
  return out << v.x() << ' ' << v.y() << ' ' << v.z();
}

// Multiply with a scalar
vec3 operator*(double t, const vec3 &v) {
  return vec3(t * v.x(), t * v.y(), t * v.z());
}
// Multiply with a scalar
vec3 operator*(const vec3 &v, double t) { return t * v; }

// Divide by a scalar
vec3 operator/(const vec3 &v, double t) {
  if (std::fabs(t) < 1e-8) {
    throw std::runtime_error("Division by zero in vec3");
  }
  return (1 / t) * v;
}

// Dot product
double dot(const vec3 &u, const vec3 &v) {
  return u.x() * v.x() + u.y() * v.y() + u.z() * v.z();
}

// Cross product
vec3 cross(const vec3 &u, const vec3 &v) {
  return vec3(u.y() * v.z() - u.z() * v.y(), u.z() * v.x() - u.x() * v.z(),
              u.x() * v.y() - u.y() * v.x());
}

// Unit vector
vec3 unit_vector(vec3 v) {
  double len = v.length();
  if (len < 1e-8) {
    throw std::runtime_error("Cannot normalize zero-length vector");
  }
  return v / v.length();
}

// Generate random unit vector
vec3 random_unit_vector() {
  while (true) {
    const auto p = vec3::random(-1, 1);
    const double squared_length = p.length_squared();
    // If too small, try again
    if (squared_length < 1e-160) {
      continue;
    }
    // If not in a unit sphere, try again
    if (squared_length > 1) {
      continue;
    }
    // Return the unit vector
    return p / std::sqrt(squared_length);
  }
}

// Generate unit vector is in the correct hemisphere based on the normal vector
vec3 random_in_hemisphere(const vec3 &normal) {
  const vec3 on_unit_square = random_unit_vector();
  // Dot product to check if the vector is in the same hemisphere
  if (dot(on_unit_square, normal) > 0.0) {
    // Positive dot product, return the vector
    return on_unit_square;
  }
  // Negative dot product, return the negative vector
  return -on_unit_square;
}

// Reflect the vector v around the normal n
// v: incident vector
// n: normal vector
vec3 reflect(const vec3 &v, const vec3 &n) {
  // dot(v, n): dot product of v and n (a scalar, negative)
  // dot(v, n) * n: projection of v onto n (a vector), which points into the
  // surface
  // -2 * dot(v, n) * n: twice the projection of v onto n (a vector), which
  // points out of the surface
  // v - 2 * dot(v, n) * n: the reflection of v around n
  return v - 2 * dot(v, n) * n;
}

// Refract the vector uv around the normal n, with the refractive index
// etai_over_etat
vec3 refract(const vec3 &uv, const vec3 &n, double etai_over_etat) {
  const double cos_theta = fmin(dot(-uv, n), 1.0);
  const vec3 r_out_perpendicular = etai_over_etat * (uv + cos_theta * n);
  const vec3 r_out_parallel =
      -std::sqrt(std::fabs(1.0 - r_out_perpendicular.length_squared())) * n;
  return r_out_perpendicular + r_out_parallel;
}