#include <iostream>

#include "utils/color.h"

int main() {

  // Image

  int image_width = 256;
  int image_height = 256;

  // Render

  std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

  for (int j = 0; j < image_height; j++) {
    std::clog << "\rScanlines remaining: " << (image_height - j) << ' '
              << std::flush;
    for (int i = 0; i < image_width; i++) {
      const auto pixel_color = color(double(i) / (image_width - 1),
                                     double(j) / (image_height - 1), 0.5);
      // Note: Write the translated [0,255] value of each color component.
      write_color(std::cout, pixel_color);
    }
  }

  std::clog << "\rDone.                 \n";
}