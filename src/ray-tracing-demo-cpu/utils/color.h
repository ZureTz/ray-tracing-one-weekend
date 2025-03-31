#pragma once

#include <algorithm>
#include <ostream>

#include "vec3.h"

// Color type alias
using color = vec3;

// Write a pixel of color to the output stream
inline void write_color(std::ostream &os, const color &pixel_color) {
  // Clamp the color values to the range [0, 1]
  auto restrict_value = [](double value) -> double {
    return std::clamp(value, 0.0, 1.0);
  };

  auto r = restrict_value(pixel_color.x());
  auto g = restrict_value(pixel_color.y());
  auto b = restrict_value(pixel_color.z());

  const int r_byte = static_cast<int>(255.999 * r);
  const int g_byte = static_cast<int>(255.999 * g);
  const int b_byte = static_cast<int>(255.999 * b);

  os << r_byte << ' ' << g_byte << ' ' << b_byte << '\n';
}
