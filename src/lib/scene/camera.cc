#include <fstream>
#include <iostream>

#include <toml++/toml.hpp>

#include "../../include/scene/camera.h"
#include "../../include/utils/rtweekend.h"

// Constructor
camera::camera(const toml::table &config) {
  // Initialize camera parameters
  initialize(config);
}

void camera::initialize(const toml::table &config) {
  // Image

  const auto aspect_ratio_width =
      config["Image"]["aspect_ratio_width"].as_floating_point()->get();
  const auto aspect_ratio_height =
      config["Image"]["aspect_ratio_height"].as_floating_point()->get();
  aspect_ratio = aspect_ratio_width / aspect_ratio_height;

  image_width = config["Image"]["image_width"].as_integer()->get();
  // Ensure that image_height is at least 1 to avoid division by zero
  image_height = std::max(int(image_width / aspect_ratio), 1);

  // Camera

  // Focal length is the distance from the camera to the viewport
  focal_length = config["Camera"]["focal_length"].as_floating_point()->get();
  viewport_height =
      config["Camera"]["viewport_height"].as_floating_point()->get();

  // Use image width / image height instead of aspect_ratio to match the image
  // Aspect ratio does not always match the viewport aspect ratio
  viewport_width = viewport_height * double(image_width) / double(image_height);
  camera_center = point3(*config["Camera"]["camera_center"].as_array());

  // Calculate the vectors across the horizontal and down the vertical viewport
  // edges.
  viewport_u = vec3(viewport_width, 0, 0);
  viewport_v = vec3(0, -viewport_height, 0);

  // Calculate the horizontal and vertical delta vectors from pixel to pixel
  pixel_u = viewport_u / double(image_width);
  pixel_v = viewport_v / double(image_height);

  // Calculate the location of the upper left pixel
  viewport_upper_left = camera_center - vec3(0, 0, focal_length) -
                        (viewport_u / 2) - (viewport_v / 2);
  pixel00_location = viewport_upper_left + 0.5 * (pixel_u + pixel_v);

  // Samples per pixel
  samples_per_pixel = config["Camera"]["samples_per_pixel"].as_integer()->get();
  // Scale for pixel samples (1 / samples_per_pixel)
  pixel_samples_scale = 1.0 / double(samples_per_pixel);

  // Background color
  background_colors = std::unordered_map<std::string, color>{
      {"white", color(*config["Color"]["white"].as_array())},
      {"blue", color(*config["Color"]["blue"].as_array())},
  };
}

// Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square
vec3 camera::sample_square() const {
  return vec3(random_double() - 0.5, random_double() - 0.5, 0);
}

// Construct a camera ray originating from the origin and directed at randomly
// sampled point around the pixel location i, j
ray camera::get_ray(int i, int j) const {
  // Offset the pixel location by a random point in the unit square
  const auto offset = sample_square();
  // Calculate the ray direction for the pixel at (i,j)
  const auto pixel_center = pixel00_location + (i + offset.x()) * pixel_u +
                            (j + offset.y()) * pixel_v;
  const auto ray_origin = camera_center;
  const auto ray_direction = pixel_center - ray_origin;

  // Return the ray
  return ray(ray_origin, ray_direction);
}

void camera::render(const hittable &world, std::ofstream &output_file) const {
  // Render

  output_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";

  for (int j = 0; j < image_height; j++) {
    std::clog << "\rScanlines remaining: " << (image_height - j) << ' '
              << std::flush;
    for (int i = 0; i < image_width; i++) {
      // Using multiple samples per pixel
      color average_color(0, 0, 0);
      for (int sample = 0; sample < samples_per_pixel; sample++) {
        // Create a ray from the camera to the pixel
        const auto r = get_ray(i, j);
        // Add sample color to the average color
        average_color += ray_color(r, world);
      }
      average_color *= pixel_samples_scale;
      write_color(output_file, average_color);
    }
  }

  std::clog << "\rDone.                 \n";
}

// Ray color for each pixel
color camera::ray_color(const ray &r, const hittable &world) const {
  // If hits draw color map

  // Check if the ray hits the sphere
  hit_record record;
  if (world.hit(r, interval(0.0, infinity), record)) {
    // Convert each component of the normal vector to a color
    // Note: The color is in the range [0, 1] and components are in the range of
    // [-1, 1], which is why we add 1 and divide by 2
    return 0.5 * (record.normal + color(1, 1, 1));
  }

  // Otherwise, draw a gradient from blue to white

  // Convert direction of ray to unit vector
  const vec3 unit_direction = unit_vector(r.direction());
  // Convert from range [-1, 1] to [0, 1] then calculate the color ratio
  const auto blend_ratio = 0.5 * (unit_direction.y() - (-1.0));

  // Blue-to-white gradient
  const auto white = background_colors.at("white");
  const auto blue = background_colors.at("blue");
  return (1 - blend_ratio) * white + blend_ratio * blue;
}