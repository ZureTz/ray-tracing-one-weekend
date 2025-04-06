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
  double aspect_ratio;
  int image_width, image_height;
  double focal_length;
  double viewport_width, viewport_height;

  point3 camera_center;
  vec3 viewport_u, viewport_v;
  vec3 pixel_u, pixel_v;
  vec3 viewport_upper_left;
  vec3 pixel00_location;

  // Background colors
  std::unordered_map<std::string, color> background_colors;
  
  // Called by the constructor
  void initialize(const toml::table &config);

  // Ray color for each pixel
  color ray_color(const ray &r, const hittable &world) const;

public:
  // Reading from a config file
  camera(const toml::table &config);

  // Render the scene
  void render(const hittable &world, std::ofstream &output_file) const;
};