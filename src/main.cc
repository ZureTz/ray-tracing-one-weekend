#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include <argparse/argparse.hpp>
#include <toml++/toml.hpp>

#include "include/hittables/hittable.h"
#include "include/hittables/hittable_list.h"
#include "include/hittables/sphere.h"
#include "include/utils/color.h"
#include "include/utils/ray.h"
#include "include/utils/rtweekend.h"
#include "include/utils/vec3.h"

color ray_color(
    const ray &r, const hittable &world,
    const std::unordered_map<std::string, color> &background_colors) {
  // If hits draw color map

  // Check if the ray hits the sphere
  hit_record record;
  if (world.hit(r, 1.0, infinity, record)) {
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

int main(int argc, char const *argv[]) {
  // Init argparse
  argparse::ArgumentParser program("ray-tracing-demo-cpu");
  // Add argument "--working-directory"
  program.add_argument("--working-directory")
      .help("Path to the working directory")
      .default_value(std::string("."))
      .append();
  // Check if the user provided a workdir
  try {
    // Example: ./ray-tracing-demo-cpu --working-directory=/path/to/dir
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  // Get the workdir and print it
  const auto workdir = program.get<std::string>("--working-directory");
  std::clog << "Working directory: " << workdir << "\n\n";

  // Load config.toml using toml++ library
  toml::table config;
  try {
    config = toml::parse_file(workdir + "/config.toml");
  } catch (std::exception e) {
    std::cerr << "Error loading config.toml: " << workdir + "/config.toml"
              << "\n";
    return 1;
  }

  // Print config.toml using toml++ library
  std::clog << "Config file content:\n\n" << config << "\n\n";

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

  // Objects (World)

  // Create world
  hittable_list world;

  // Create a sphere object
  const auto center = point3(*config["Sphere"]["center"].as_array());
  const auto radius = config["Sphere"]["radius"].as_floating_point()->get();
  // Add sphere to the world
  world.add(std::make_shared<sphere>(center, radius));

  // Background color
  const std::unordered_map<std::string, color> background_colors{
      {"white", color(*config["Color"]["white"].as_array())},
      {"blue", color(*config["Color"]["blue"].as_array())},
  };

  // Render
  // Open output file
  std::ofstream output_file(workdir + "/output/output.ppm");

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
      const auto pixel_color = ray_color(r, world, background_colors);
      // Note: Write the translated [0,255] value of each color component
      write_color(output_file, pixel_color);
    }
  }

  std::clog << "\rDone.                 \n";
}