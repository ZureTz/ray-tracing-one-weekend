#include <ostream>

#include "../../include/utils/color.h"
#include "../../include/utils/interval.h"

// Write a pixel of color to the output stream
void write_color(std::ostream &os, const color &pixel_color) {
  // Extract the color components from the pixel color.
  auto r = pixel_color.x();
  auto g = pixel_color.y();
  auto b = pixel_color.z();

  // Translate the [0,1] component values to the byte range [0,255].
  static const interval intensity(0.0000, 0.9999);
  const int r_byte = static_cast<int>(255.999 * intensity.clamp(r));
  const int g_byte = static_cast<int>(255.999 * intensity.clamp(g));
  const int b_byte = static_cast<int>(255.999 * intensity.clamp(b));

  // Write the translated [0,255] value to the output stream.
  os << r_byte << ' ' << g_byte << ' ' << b_byte << '\n';
}
