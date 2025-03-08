#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

void render_rows_parallel(const int start_row, const int end_row,
                          const int image_width, const int image_height,
                          std::vector<std::string> &output_buffer,
                          std::mutex &progress_mutex, int &progress) {
  for (int j = start_row; j < end_row; j++) {
    std::stringstream row_ss;

    {
      const std::lock_guard<std::mutex> lock(progress_mutex);
      std::clog << "\rScanlines: " << (progress + 1) << '/' << image_height
                << std::flush;
      progress++;
    }

    for (int i = 0; i < image_width; i++) {
      auto r_factor = double(i) / (image_width - 1);
      auto g_factor = double(j) / (image_height - 1);
      auto b_factor = 0.5;

      const int ir = int(255.999 * r_factor);
      const int ig = int(255.999 * g_factor);
      const int ib = int(255.999 * b_factor);

      row_ss << ir << ' ' << ig << ' ' << ib << '\n';
    }

    // Put result into the buffer
    {
      const std::lock_guard<std::mutex> lock(progress_mutex);
      output_buffer[j - start_row] = row_ss.str();
    }
  }
}

int main() {
  // Image
  const int image_width = 1920;
  const int image_height = 1080;

  // Thread count
  constexpr int thread_count = 16;
  std::clog << "Using " << thread_count << " threads.\n";

  // Thread array
  std::vector<std::thread> threads(thread_count);

  // Create Buffer
  std::vector<std::vector<std::string>> buffers(thread_count);
  // Also Mutex for counting scanlines
  std::mutex progress_mutex;

  const int rows_per_thread = image_height / thread_count;

  // Render

  // Start threads
  for (int t = 0, progress = 0; t < thread_count; t++) {
    const int start_row = t * rows_per_thread;
    const int end_row =
        (t == thread_count - 1) ? image_height : start_row + rows_per_thread;

    buffers[t].resize(end_row - start_row);
    threads[t] = std::thread(render_rows_parallel, start_row, end_row,
                             image_width, image_height, std::ref(buffers[t]),
                             std::ref(progress_mutex), std::ref(progress));
  }

  // Log the start of the output
  std::clog << "\rWriting PPM file...               \n";

  // Print the ppm info to stdout (which can be redirected to a file)
  std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

  // Wait for threads to finish
  for (int t = 0; t < thread_count; t++) {
    threads[t].join();
  }

  // Print the image data from the buffers
  for (int t = 0; t < thread_count; t++) {
    for (const auto &row : buffers[t]) {
      std::cout << row;
    }
  }

  std::clog << "\rDone.                       \n";
}