#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <toml++/toml.hpp>

#include "hittables/hittable.h"
#include "hittables/material.h"
#include "scene/camera.h"
#include "utils/rtweekend.h"
#include "utils/vec3.h"

// Constructor
camera::camera(const toml::table &config) {
  try {
    // Initialize camera parameters
    initialize(config);
  } catch (const std::exception &e) {
    std::cerr << "相机初始化失败: " << e.what() << "\n";
    exit(EXIT_FAILURE);
  }
}

void camera::initialize(const toml::table &config) {
  try {
    // 检查所需配置项是否存在
    if (!config.contains("Image") || !config["Image"].is_table() ||
        !config.contains("Camera") || !config["Camera"].is_table() ||
        !config.contains("Color") || !config["Color"].is_table() ||
        !config.contains("Ray") || !config["Ray"].is_table()) {
      throw std::runtime_error(
          "缺少必要的配置部分: Image, Camera, Color 或 Ray");
    }

    // Image 部分验证
    if (!config["Image"].as_table()->contains("aspect_ratio_width") ||
        !config["Image"].as_table()->contains("aspect_ratio_height") ||
        !config["Image"].as_table()->contains("image_width")) {
      throw std::runtime_error("缺少 Image 部分的必要配置项");
    }

    // 获取并验证宽高比
    const auto aspect_ratio_width =
        config["Image"]["aspect_ratio_width"].as_floating_point();
    const auto aspect_ratio_height =
        config["Image"]["aspect_ratio_height"].as_floating_point();

    if (!aspect_ratio_width || !aspect_ratio_height) {
      throw std::runtime_error("宽高比必须是浮点数");
    }

    double width_val = aspect_ratio_width->get();
    double height_val = aspect_ratio_height->get();

    if (width_val <= 0 || height_val <= 0) {
      throw std::runtime_error("宽高比必须为正数");
    }

    const double aspect_ratio = width_val / height_val;

    // 获取并验证图像宽度
    const auto width_node = config["Image"]["image_width"].as_integer();
    if (!width_node) {
      throw std::runtime_error("图像宽度必须是整数");
    }

    image_width = width_node->get();
    if (image_width <= 0) {
      throw std::runtime_error("图像宽度必须为正整数");
    }

    // Ensure that image_height is at least 1 to avoid division by zero
    image_height = std::max(int(image_width / aspect_ratio), 1);

    // Camera 部分验证
    if (!config["Camera"].as_table()->contains("v_fov") ||
        !config["Camera"].as_table()->contains("look_from") ||
        !config["Camera"].as_table()->contains("look_at") ||
        !config["Camera"].as_table()->contains("vup") ||
        !config["Camera"].as_table()->contains("samples_per_pixel")) {
      throw std::runtime_error("缺少 Camera 部分的必要配置项");
    }

    // Get v_fov node and validate it
    const auto v_fov_node = config["Camera"]["v_fov"].as_floating_point();
    if (!v_fov_node) {
      throw std::runtime_error("v_fov必须是浮点数");
    }

    const auto v_fov = v_fov_node->get();
    if (v_fov <= 0 || v_fov >= 180) {
      throw std::runtime_error("v_fov必须在0到180之间");
    }

    // 获取并验证look_from
    const auto look_from_node = config["Camera"]["look_from"].as_array();
    if (!look_from_node || look_from_node->size() != 3) {
      throw std::runtime_error("look_from必须是包含3个元素的数组");
    }

    const vec3 look_from(*look_from_node);
    camera_center = look_from;

    // 获取并验证look_at
    const auto look_at_node = config["Camera"]["look_at"].as_array();
    if (!look_at_node || look_at_node->size() != 3) {
      throw std::runtime_error("look_at必须是包含3个元素的数组");
    }
    const vec3 look_at(*look_at_node);

    // 获取并验证vup
    const auto vup_node = config["Camera"]["vup"].as_array();
    if (!vup_node || vup_node->size() != 3) {
      throw std::runtime_error("vup必须是包含3个元素的数组");
    }
    const vec3 vup(*vup_node);

    // Calculate the focal length
    const auto focal_length = (look_from - look_at).length();
    // Calculate the viewport height based on the vertical field of view
    // Convert degrees to radians
    const double theta = degrees_to_radians(v_fov);
    // Calculate the half height of the viewport
    const double half_height = std::tan(theta / 2);
    // Calculate the viewport height
    const double viewport_height = 2.0 * half_height * focal_length;

    // Use image width / image height instead of aspect_ratio to match the image
    // Aspect ratio does not always match the viewport aspect ratio
    const double viewport_width =
        viewport_height * double(image_width) / double(image_height);

    // Calculate the u, v, w unit basis vectors for the camera coordinate frame
    w = unit_vector(look_from - look_at);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    // Calculate the vectors across the horizontal and down the vertical
    // viewport edges.
    const auto viewport_u = viewport_width * u;
    const auto viewport_v = viewport_height * -v;

    // Calculate the horizontal and vertical delta vectors from pixel to pixel
    pixel_u = viewport_u / double(image_width);
    pixel_v = viewport_v / double(image_height);

    // Calculate the location of the upper left pixel
    const auto viewport_upper_left = camera_center - (focal_length * w) -
                                     (0.5 * viewport_u) - (0.5 * viewport_v);
    pixel00_location = viewport_upper_left + 0.5 * (pixel_u + pixel_v);

    // 获取并验证采样数
    const auto samples_node =
        config["Camera"]["samples_per_pixel"].as_integer();
    if (!samples_node) {
      throw std::runtime_error("每像素采样数必须是整数");
    }

    samples_per_pixel = samples_node->get();
    if (samples_per_pixel <= 0) {
      throw std::runtime_error("每像素采样数必须为正整数");
    }

    // Scale for pixel samples (1 / samples_per_pixel)
    pixel_samples_scale = 1.0 / double(samples_per_pixel);

    // Color 部分验证
    if (!config["Color"].as_table()->contains("white") ||
        !config["Color"].as_table()->contains("blue")) {
      throw std::runtime_error("缺少 Color 部分的必要配置项");
    }

    // 获取并验证背景颜色
    const auto white_node = config["Color"]["white"].as_array();
    const auto blue_node = config["Color"]["blue"].as_array();

    if (!white_node || white_node->size() != 3 || !blue_node ||
        blue_node->size() != 3) {
      throw std::runtime_error("颜色必须是包含3个元素的数组");
    }

    color white = color(*white_node);
    color blue = color(*blue_node);

    // 验证颜色值是否在有效范围内 [0,1]
    if (white.x() < 0 || white.x() > 1 || white.y() < 0 || white.y() > 1 ||
        white.z() < 0 || white.z() > 1 || blue.x() < 0 || blue.x() > 1 ||
        blue.y() < 0 || blue.y() > 1 || blue.z() < 0 || blue.z() > 1) {
      throw std::runtime_error("颜色值必须在范围 [0,1] 内");
    }

    // Convert from gamma to linear space
    white = color(white.x() * white.x(), white.y() * white.y(),
                  white.z() * white.z());
    blue = color(blue.x() * blue.x(), blue.y() * blue.y(), blue.z() * blue.z());

    background_colors = std::unordered_map<std::string, color>{
        {"white", white},
        {"blue", blue},
    };

    // Ray 部分验证
    if (!config["Ray"].as_table()->contains("max_depth")) {
      throw std::runtime_error("缺少 Ray 部分的必要配置项");
    }

    // 获取并验证最大光线深度
    const auto max_depth_node = config["Ray"]["max_depth"].as_integer();
    if (!max_depth_node) {
      throw std::runtime_error("最大光线深度必须是整数");
    }

    max_depth = max_depth_node->get();
    if (max_depth <= 0) {
      throw std::runtime_error("最大光线深度必须为正整数");
    }
  } catch (const toml::parse_error &e) {
    throw std::runtime_error("TOML解析错误: " + std::string(e.what()));
  } catch (const std::exception &e) {
    throw std::runtime_error("配置验证错误: " + std::string(e.what()));
  }
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

// Single threaded render function
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
        average_color += ray_color(r, max_depth, world);
      }
      average_color *= pixel_samples_scale;
      write_color(output_file, average_color);
    }
  }

  std::clog << "\rDone.                 \n";
}

// Multithreaded render function
void camera::render_multithread(const hittable &world,
                                std::ofstream &output_file) const {
  // Threads
  const int num_threads = std::thread::hardware_concurrency();

  // Thread array
  std::vector<std::thread> threads(num_threads);

  // Create Buffer
  std::vector<std::string> buffers(num_threads);

  // Also Mutex for counting scanlines
  std::mutex progress_mutex;

  const int rows_per_thread = image_height / num_threads;

  auto render_rows_parallel =
      [this](const int start_row, const int end_row, const hittable &world,
             std::string &output_buffer, std::mutex &progress_mutex,
             int &progress) -> void {
    // Stringstream to store the result
    std::stringstream result;
    for (int j = start_row; j < end_row; j++) {
      {
        const std::lock_guard<std::mutex> lock(progress_mutex);
        std::clog << "\rScanlines: " << (progress + 1) << '/' << image_height
                  << std::flush;
        progress++;
      }

      for (int i = 0; i < image_width; i++) {
        // Using multiple samples per pixel
        color average_color(0, 0, 0);
        for (int sample = 0; sample < samples_per_pixel; sample++) {
          // Create a ray from the camera to the pixel
          const auto r = get_ray(i, j);
          // Add sample color to the average color
          average_color += ray_color(r, max_depth, world);
        }
        average_color *= pixel_samples_scale;
        write_color(result, average_color);
      }
    }

    // Add the result to the output buffer
    output_buffer += result.str();
  };

  // Render

  // Start threads
  for (int t = 0, progress = 0; t < num_threads; t++) {
    const int start_row = t * rows_per_thread;
    const int end_row =
        (t == num_threads - 1) ? image_height : start_row + rows_per_thread;
    threads[t] = std::thread(render_rows_parallel, start_row, end_row,
                             std::ref(world), std::ref(buffers[t]),
                             std::ref(progress_mutex), std::ref(progress));
  }

  output_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";

  // Wait for threads to finish
  for (int t = 0; t < num_threads; t++) {
    threads[t].join();
  }

  // Print the image data from the buffers
  for (int t = 0; t < num_threads; t++) {
    output_file << buffers[t];
  }

  std::clog << "\rDone.                 \n";
}

// Ray color for each pixel
color camera::ray_color(const ray &r, const int depth,
                        const hittable &world) const {
  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth <= 0) {
    return color(0, 0, 0);
  }

  // If hits draw color map

  // Check if the ray hits the sphere
  hit_record record;
  // Use 0.001 as the minimum distance to avoid self-intersection (Causing
  // shadow acne)
  if (world.hit(r, interval(0.001, infinity), record)) {
    // Create scattered ray and attenuation color
    ray scattered;
    color attenuation;
    // And scatter the ray based on the material
    const material &mat = *record.mat;
    if (mat.scatter(r, record, attenuation, scattered)) {
      // Return the color of the scattered ray
      return attenuation * ray_color(scattered, depth - 1, world);
    }
    // If the ray is absorbed, return black
    return color(0, 0, 0);
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