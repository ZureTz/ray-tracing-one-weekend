#pragma once
// Camera class, responsible for rendering the scene

#include <string>
#include <toml++/toml.hpp>
#include <unordered_map>

#include "hittables/hittable.h"
#include "utils/color.h"

class camera {
private:
  // Camera parameters
  int image_width, image_height; // Rendered image width and height
  point3 camera_center;          // Camera position in 3D space
  vec3 u, v, w;                  // Camera frame basis vectors
  vec3 pixel_u, pixel_v;         // Pixel vectors
  vec3 pixel00_location;         // Location of the first pixel

  int samples_per_pixel;      // Sample per pixel for anti-aliasing
  double pixel_samples_scale; // Scale for pixel samples (1 / samples_per_pixel)

  // Background colors
  std::unordered_map<std::string, color> background_colors;

  // Max ray bounce depth
  int max_depth;

  // Called by the constructor
  void initialize(const toml::table &config);

  // Ray color for each pixel
  color ray_color(const ray &r, const int depth, const hittable &world) const;

  // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square
  vec3 sample_square() const;

  // Construct a camera ray originating from the origin and directed at randomly
  // sampled point around the pixel location i, j
  ray get_ray(int i, int j) const;

public:
  // Reading from a config file
  camera(const toml::table &config);

  // Render the scene
  void render(const hittable &world, std::ofstream &output_file) const;

  // Multithreaded render function
  void render_multithread(const hittable &world,
                          std::ofstream &output_file) const;
};