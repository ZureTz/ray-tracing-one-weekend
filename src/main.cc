#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <argparse/argparse.hpp>
#include <toml++/toml.hpp>

#include "hittables/hittable.h"
#include "hittables/hittable_list.h"
#include "hittables/material.h"
#include "hittables/sphere.h"
#include "scene/camera.h"
#include "utils/color.h"
#include "utils/vec3.h"

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
  } catch (toml::parse_error &err) {
    std::cerr << "Error loading config.toml: " << workdir + "/config.toml"
              << "\n";
    std::cerr << err << "\n";
    return 1;
  }

  // Print config.toml using toml++ library
  std::clog << "Config file content:\n\n" << config << "\n\n";

  // Objects (World)

  // Create world
  hittable_list world;

  // Create a sphere object list
  const auto config_spheres = *config["Sphere"].as_array();

  // Create material alias (from string to material class)
  auto config_to_material =
      [](const toml::table conf) -> std::shared_ptr<material> {
    // Get the material type and albedo
    const auto material_type = conf["material"].as_string()->get();
    const auto albedo = color(*conf["albedo"].as_array());

    if (material_type == "lambertian") {
      return std::make_shared<lambertian>(albedo);
    }
    if (material_type == "metal") {
      const double fuzz =
          conf.contains("fuzz") ? conf["fuzz"].as_floating_point()->get() : 0.0;
      return std::make_shared<metal>(albedo, fuzz);
    }
    // Invalid type
    return nullptr;
  };

  // For each spheres in the list
  for (const auto &s : config_spheres) {
    // Convert s to table
    const auto s_table = *s.as_table();

    // Create the material
    const auto mat = config_to_material(s_table);
    // Check if the material is valid
    if (mat == nullptr) {
      std::cerr << "Invalid material type: "
                << s_table["material"].as_string()->get() << "\n";
      return 1;
    }

    // Get the center and radius of the sphere
    const auto center = point3(*s_table["center"].as_array());
    const auto radius = s_table["radius"].as_floating_point()->get();

    // Add sphere to the world
    world.add(std::make_shared<sphere>(center, radius, mat));
  }

  // Open output file
  std::ofstream output_file(workdir + "/output/output.ppm");
  // Then render
  camera cam(config);
  // cam.render(world, output_file);
  cam.render_multithread(world, output_file);

  return 0;
}