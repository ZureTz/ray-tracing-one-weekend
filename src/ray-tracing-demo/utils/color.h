#pragma once

#include <ostream>

#include "vec3.h"

// Color type alias
using color = vec3;

// Write a pixel of color to the output stream
inline void write_color(std::ostream &os, const color &pixel_color) {
  auto r = pixel_color.x();
  auto g = pixel_color.y();
  auto b = pixel_color.z();

  const int r_byte = static_cast<int>(255.999 * r);
  const int g_byte = static_cast<int>(255.999 * g);
  const int b_byte = static_cast<int>(255.999 * b);

  os << r_byte << ' ' << g_byte << ' ' << b_byte << '\n';
}
