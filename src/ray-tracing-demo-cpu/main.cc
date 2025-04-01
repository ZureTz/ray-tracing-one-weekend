#include <algorithm>
#include <iostream>

#include "utils/color.h"
#include "utils/ray.h"
#include "utils/vec3.h"

// Determine if the ray hits the sphere
double hit_sphere(const point3 &center, const double radius, const ray &r) {
  // t^2⋅d⋅d−2t⋅d⋅(C−Q)+(C−Q)⋅(C−Q)−r^2=0
  // a = d⋅d
  // b = -2⋅d⋅(C−Q); h = d⋅(C−Q)
  // c = (C−Q)⋅(C−Q)−r^2

  const auto d = r.direction();
  const auto C_Q = center - r.origin();

  const auto a = d.length_squared();
  const auto h = dot(d, C_Q);
  const auto c = C_Q.length_squared() - radius * radius;

  // Determine if it has root: b^2 - 4ac >= 0
  const auto discriminant = h * h - a * c;

  // If not hit
  if (discriminant < 0) {
    return -1.0;
  }

  // If hit, return the nearest root
  return (h - std::sqrt(discriminant)) / a;
}

// Default ray color to 0,0,0
color ray_color(const ray &r) {
  // If hits sphere draw color map
  const auto center = point3(0.5, 0.2, -1.2);
  const auto radius = 0.5;

  // Check if the ray hits the sphere
  const auto solved_t = hit_sphere(center, radius, r);
  if (solved_t > 0) {
    const vec3 normal = unit_vector(r.at(solved_t) - center);
    // Convert each component of the normal vector to a color
    // Note: The color is in the range [0, 1] and components are in the range of
    // [-1, 1], which is why we add 1 and divide by 2
    return 0.5 * color(normal.x() + 1, normal.y() + 1, normal.z() + 1);
  }

  // Otherwise, draw a gradient from blue to white

  // Convert direction of ray to unit vector
  const vec3 unit_direction = unit_vector(r.direction());
  // Convert from range [-1, 1] to [0, 1] then calculate the color ratio
  const auto blend_ratio = 0.5 * (unit_direction.y() - (-1.0));

  // Blue-to-white gradient
  const auto white = color(1.0, 1.0, 1.0);
  const auto blue = color(0.5, 0.7, 1.0);
  return (1 - blend_ratio) * white + blend_ratio * blue;
}

int main() {

  // Image

  const auto aspect_ratio = 16.0 / 9.0;
  const int image_width = 720;
  // Ensure that image_height is at least 1 to avoid division by zero
  const int image_height = std::max(int(image_width / aspect_ratio), 1);

  // Camera

  // Focal length is the distance from the camera to the viewport
  const auto focal_length = 1.0;
  const auto viewport_height = 2.0;
  // Use image width / image height instead of aspect_ratio to match the image
  // Aspect ratio does not always match the viewport aspect ratio
  const auto viewport_width =
      viewport_height * double(image_width) / double(image_height);
  const auto camera_center = point3(0, 0, 0);

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

  // Render

  std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

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

      // A dummy color for test
      const auto pixel_color = ray_color(r);
      // Note: Write the translated [0,255] value of each color component
      write_color(std::cout, pixel_color);
    }
  }

  std::clog << "\rDone.                 \n";
}