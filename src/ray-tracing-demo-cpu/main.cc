#include <algorithm>
#include <fstream>
#include <iostream>

#include <toml++/toml.hpp>

#include "hittables/hittable.h"
#include "hittables/sphere.h"
#include "utils/color.h"
#include "utils/ray.h"
#include "utils/vec3.h"

// Load config.toml using toml++ library
const toml::table config = toml::parse_file("config.toml");

// Colors
const auto white = color(*config["Color"]["white"].as_array());
const auto blue = color(*config["Color"]["blue"].as_array());

// Default ray color to 0,0,0
color ray_color(const ray &r, const hittable &h) {
  // If hits draw color map

  // Check if the ray hits the sphere
  hit_record record;
  if (h.hit(r, 0.0, std::numeric_limits<double>::max(), record)) {
    // Convert each component of the normal vector to a color
    // Note: The color is in the range [0, 1] and components are in the range of
    // [-1, 1], which is why we add 1 and divide by 2
    return 0.5 * color(record.normal.x() + 1, record.normal.y() + 1,
                       record.normal.z() + 1);
  }

  // Otherwise, draw a gradient from blue to white

  // Convert direction of ray to unit vector
  const vec3 unit_direction = unit_vector(r.direction());
  // Convert from range [-1, 1] to [0, 1] then calculate the color ratio
  const auto blend_ratio = 0.5 * (unit_direction.y() - (-1.0));

  // Blue-to-white gradient
  return (1 - blend_ratio) * white + blend_ratio * blue;
}

int main() {
  // Print config.toml using toml++ library
  std::clog << config << "\n\n";

  // Image

  const auto aspect_ratio_width =
      config["Image"]["aspect_ratio_width"].as_floating_point()->get();
  const auto aspect_ratio_height =
      config["Image"]["aspect_ratio_height"].as_floating_point()->get();
  const auto aspect_ratio = aspect_ratio_width / aspect_ratio_height;

  const int image_width = config["Image"]["image_width"].as_integer()->get();
  // Ensure that image_height is at least 1 to avoid division by zero
  const int image_height = std::max(int(image_width / aspect_ratio), 1);

  // Camera

  // Focal length is the distance from the camera to the viewport
  const auto focal_length =
      config["Camera"]["focal_length"].as_floating_point()->get();
  const auto viewport_height =
      config["Camera"]["viewport_height"].as_floating_point()->get();

  // Use image width / image height instead of aspect_ratio to match the image
  // Aspect ratio does not always match the viewport aspect ratio
  const auto viewport_width =
      viewport_height * double(image_width) / double(image_height);
  const auto camera_center =
      point3(*config["Camera"]["camera_center"].as_array());

  // Calculate the vectors across the horizontal and down the vertical viewport
  // edges.
  const auto viewport_u = vec3(viewport_width, 0, 0);
  const auto viewport_v = vec3(0, -viewport_height, 0);

  // Calculate the horizontal and vertical delta vectors from pixel to pixel
  const auto pixel_u = viewport_u / double(image_width);
  const auto pixel_v = viewport_v / double(image_height);

  // Calculate the location of the upper left pixel
  const auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) -
                                   (viewport_u / 2) - (viewport_v / 2);
  const auto pixel00_location = viewport_upper_left + 0.5 * (pixel_u + pixel_v);

  // Objects

  // Create a sphere object
  const auto center = point3(*config["Sphere"]["center"].as_array());
  const auto radius = config["Sphere"]["radius"].as_floating_point()->get();
  const sphere s(center, radius);

  // Render
  // Open output file
  std::ofstream output_file("output/output.ppm");

  output_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";

  for (int j = 0; j < image_height; j++) {
    std::clog << "\rScanlines remaining: " << (image_height - j) << ' '
              << std::flush;
    for (int i = 0; i < image_width; i++) {
      // Calculate the ray direction for the pixel at (i,j)
      const auto pixel_center =
          pixel00_location + (i * pixel_u) + (j * pixel_v);
      const auto ray_direction = pixel_center - camera_center;

      // Create a ray from the camera to the pixel
      const auto r = ray(camera_center, ray_direction);

      // Find out color
      const auto pixel_color = ray_color(r, s);
      // Note: Write the translated [0,255] value of each color component
      write_color(output_file, pixel_color);
    }
  }

  std::clog << "\rDone.                 \n";
}