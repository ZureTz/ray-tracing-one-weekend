#pragma once

#include <array>
#include <cmath>
#include <ostream>

#include <toml++/toml.hpp>

class vec3 {
  // A array with fixed length 3
  std::array<double, 3> e;

public:
  // Initializers
  vec3() : e{0, 0, 0} {}
  vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}
  vec3(const toml::array &arr) {
    if (arr.size() != 3) {
      throw std::runtime_error("vec3 constructor requires a toml::array of size 3");
    }
    e[0] = arr[0].as_floating_point()->get();
    e[1] = arr[1].as_floating_point()->get();
    e[2] = arr[2].as_floating_point()->get();
  }

  // Get
  double x() const { return e[0]; }
  double y() const { return e[1]; }
  double z() const { return e[2]; }

  // Negate
  vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
  // Get specific coordinate using index (const)
  double operator[](int i) const { return e[i]; }
  // Get specific coordinate using index (non-const, reference)
  double &operator[](int i) { return e[i]; }

  // Add with others
  vec3 operator+(const vec3 &v) const {
    return vec3(e[0] + v.e[0], e[1] + v.e[1], e[2] + v.e[2]);
  }

  // Subtract with others
  vec3 operator-(const vec3 &v) const {
    return vec3(e[0] - v.e[0], e[1] - v.e[1], e[2] - v.e[2]);
  }

  // Multiply with others
  vec3 operator*(const vec3 &v) const {
    return vec3(e[0] * v.e[0], e[1] * v.e[1], e[2] * v.e[2]);
  }

  // Add together, write to self
  vec3 &operator+=(const vec3 &v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }

  // Subtract together, write to self
  vec3 &operator-=(const vec3 &v) {
    e[0] -= v.e[0];
    e[1] -= v.e[1];
    e[2] -= v.e[2];
    return *this;
  }

  // Multiply together, write to self
  vec3 &operator*=(const vec3 &v) {
    e[0] *= v.e[0];
    e[1] *= v.e[1];
    e[2] *= v.e[2];
    return *this;
  }

  // Multiply by a scalar (*=)
  vec3 &operator*=(const double t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  // Divide by a scalar (/=)
  vec3 &operator/=(const double t) {
    if (std::fabs(t) < 1e-8) {
      throw std::runtime_error("Division by zero in vec3");
    }
    return *this *= 1 / t;
  }

  // Length in square
  double length_squared() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }

  // Length
  double length() const { return std::sqrt(length_squared()); }
};

// point3 is just an alias for vec3, but useful for geometric clarity in the
// code.
using point3 = vec3;

// Vector Utility Functions

// Output vec3
inline std::ostream &operator<<(std::ostream &out, const vec3 &v) {
  return out << v.x() << ' ' << v.y() << ' ' << v.z();
}

// Multiply with a scalar
inline vec3 operator*(double t, const vec3 &v) {
  return vec3(t * v.x(), t * v.y(), t * v.z());
}
// Multiply with a scalar
inline vec3 operator*(const vec3 &v, double t) { return t * v; }

// Divide by a scalar
inline vec3 operator/(const vec3 &v, double t) {
  if (std::fabs(t) < 1e-8) {
    throw std::runtime_error("Division by zero in vec3");
  }
  return (1 / t) * v;
}

// Dot product
inline double dot(const vec3 &u, const vec3 &v) {
  return u.x() * v.x() + u.y() * v.y() + u.z() * v.z();
}

// Cross product
inline vec3 cross(const vec3 &u, const vec3 &v) {
  return vec3(u.y() * v.z() - u.z() * v.y(), u.z() * v.x() - u.x() * v.z(),
              u.x() * v.y() - u.y() * v.x());
}

// Unit vector
inline vec3 unit_vector(vec3 v) {
  double len = v.length();
  if (len < 1e-8) {
    throw std::runtime_error("Cannot normalize zero-length vector");
  }
  return v / v.length();
}