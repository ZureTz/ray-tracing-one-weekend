#pragma once
// Camera class, responsible for rendering the scene

#include <string>
#include <toml++/toml.hpp>
#include <unordered_map>

#include "../hittables/hittable.h"
#include "../utils/color.h"

class camera {
private:
  // Camera parameters
  double aspect_ratio;                    // Aspect ratio of the image
  int image_width, image_height;          // Rendered image width and height
  double focal_length;                    // Focal length of the camera
  double viewport_width, viewport_height; // Viewport dimensions

  point3 camera_center;        // Camera position in 3D space
  vec3 viewport_u, viewport_v; // Viewport vectors
  vec3 pixel_u, pixel_v;       // Pixel vectors
  vec3 viewport_upper_left;    // Upper left corner of the viewport
  vec3 pixel00_location;       // Location of the first pixel

  int samples_per_pixel;      // Sample per pixel for anti-aliasing
  double pixel_samples_scale; // Scale for pixel samples (1 / samples_per_pixel)

  // Background colors
  std::unordered_map<std::string, color> background_colors;

  // Called by the constructor
  void initialize(const toml::table &config);

  // Ray color for each pixel
  color ray_color(const ray &r, const hittable &world) const;

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
};