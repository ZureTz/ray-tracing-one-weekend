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
  std::clog << "Loading config.toml: " << workdir + "/config.toml" << "\n";
  try {
    config = toml::parse_file(workdir + "/config.toml");
  } catch (toml::parse_error &err) {
    std::cerr << "Error loading config.toml: " << workdir + "/config.toml"
              << "\n";
    std::cerr << err << "\n";
    return 1;
  }
  std::clog << "Loaded config.toml successfully.\n";

  // Objects (World)

  // Create world
  hittable_list world;

  // Create a sphere object list
  const auto config_spheres = *config["Sphere"].as_array();

  // Create material alias (from string to material class)
  auto config_to_material =
      [](const toml::table conf_object) -> std::shared_ptr<material> {
    // 检查材质配置是否完整
    if (!conf_object.contains("material") ||
        !conf_object["material"].is_string()) {
      std::cerr << "Error: Each sphere must have a valid 'material' property "
                   "of type string.\n";
      return nullptr;
    }

    // 检查albedo是否存在且是数组
    if (!conf_object.contains("albedo") || !conf_object["albedo"].is_array()) {
      std::cerr << "Error: Each sphere must have a valid 'albedo' property as "
                   "an array of three numbers.\n";
      return nullptr;
    }

    // 尝试解析albedo并检查其有效性
    const color albedo(*conf_object["albedo"].as_array());
    if (std::isnan(albedo.x()) || std::isinf(albedo.x()) ||
        std::isnan(albedo.y()) || std::isinf(albedo.y()) ||
        std::isnan(albedo.z()) || std::isinf(albedo.z())) {
      std::cerr << "Error: Albedo contains invalid values: " << albedo << "\n";
      return nullptr;
    }

    // 检查albedo颜色值是否在合理范围内(0-1)
    if (albedo.x() < 0 || albedo.x() > 1 || albedo.y() < 0 || albedo.y() > 1 ||
        albedo.z() < 0 || albedo.z() > 1) {
      std::cerr << "Warning: Albedo values should typically be in range [0,1]. "
                   "Current values: "
                << albedo << "\n";
      // 这里只是警告，不返回nullptr
    }

    // Get the material type
    const auto material_type = conf_object["material"].as_string()->get();

    if (material_type == "lambertian") {
      return std::make_shared<lambertian>(albedo);
    }

    if (material_type == "metal") {
      // 检查fuzz参数
      if (!conf_object.contains("fuzz")) {
        return std::make_shared<metal>(albedo, 0.0);
      }

      // 检查fuzz参数是否为浮点数
      const auto fuzz_node = conf_object["fuzz"].as_floating_point();
      if (!fuzz_node) {
        std::cerr << "Error: Metal material 'fuzz' parameter must be a "
                     "floating-point number.\n";
        return nullptr;
      }

      // 获取fuzz值
      const double fuzz = fuzz_node->get();

      if (std::isnan(fuzz) || std::isinf(fuzz) || fuzz < 0) {
        std::cerr << "Error: Metal material 'fuzz' parameter must be a "
                     "non-negative number.\n";
        return nullptr;
      }

      // fuzz值过大会导致渲染问题，通常应限制在0-1范围内
      if (fuzz > 1) {
        std::cerr << "Warning: Metal material 'fuzz' parameter should "
                     "typically be in range [0,1]. Current value: "
                  << fuzz << "\n";
      }
      return std::make_shared<metal>(albedo, fuzz);
    }

    if (material_type == "dielectric") {
      // 检查refractive_index参数
      if (!conf_object.contains("refractive_index")) {
        return std::make_shared<dielectric>(1.0);
      }

      // 检查refractive_index参数是否为浮点数
      const auto refractive_index_node =
          conf_object["refractive_index"].as_floating_point();
      if (!refractive_index_node) {
        std::cerr << "Error: Dielectric material 'refractive_index' must be "
                     "a floating-point number.\n";
        return nullptr;
      }

      // 获取refractive_index值
      const double refractive_index = refractive_index_node->get();

      if (std::isnan(refractive_index) || std::isinf(refractive_index) ||
          refractive_index <= 0) {
        std::cerr << "Error: Dielectric material 'refractive_index' must be "
                     "a positive number.\n";
        return nullptr;
      }
      return std::make_shared<dielectric>(refractive_index);
    }

    // Invalid type
    std::cerr << "Error: Unknown material type: '" << material_type
              << "'. Supported types: lambertian, metal, dielectric.\n";
    return nullptr;
  };

  // For each spheres in the list
  for (const auto &s : config_spheres) {
    // Convert s to table
    const auto s_table_node = s.as_table();

    // Check if the node is valid
    if (!s_table_node) {
      std::cerr << "Error: Sphere configuration is not a valid table.\n";
      return 1;
    }

    // Convert to table
    const auto s_table = *s_table_node;

    // Create the material
    const auto mat = config_to_material(s_table);
    // Check if the material is valid
    if (mat == nullptr) {
      std::cerr << "Invalid material type: "
                << s_table["material"].as_string()->get() << "\n";
      return 1;
    }

    // Get the center and radius of the sphere
    const auto center_node = s_table["center"].as_array();
    if (!center_node || center_node->size() != 3) {
      std::cerr << "Error: Sphere center must be an array of three numbers.\n";
      return 1;
    }
    // Convert to point3
    const auto center = point3(*center_node);

    // Get radius node
    const auto radius_node = s_table["radius"].as_floating_point();
    if (!radius_node) {
      std::cerr << "Error: Sphere radius must be a floating-point number.\n";
      return 1;
    }

    const auto radius = radius_node->get();

    // 添加有效性检查
    // 检查中心点坐标是否合法（不是NaN或无穷大）
    if (std::isnan(center.x()) || std::isinf(center.x()) ||
        std::isnan(center.y()) || std::isinf(center.y()) ||
        std::isnan(center.z()) || std::isinf(center.z())) {
      std::cerr << "Invalid sphere center coordinates. Center: " << center
                << "\n";
      return 1;
    }

    // 检查半径是否为正数且不是NaN或无穷大
    if (radius <= 0 || std::isnan(radius) || std::isinf(radius)) {
      std::cerr << "Invalid sphere radius: " << radius
                << ". Radius must be a positive number.\n";
      return 1;
    }

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