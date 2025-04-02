-- Generate compile_commands.json for clangd every time the project is built
add_rules("plugin.compile_commands.autoupdate", {lsp = "clangd"})
-- Set c++ code standard: c++17
set_languages("c++17")
add_requires("toml++", {alias = "tomlplusplus"})

target("generate-ppm-image-multithread")
  set_kind("binary")
  add_files("src/generate-ppm-image-multithread/*.cc")

target("ray-tracing-demo-cpu")
  set_kind("binary")
  add_files("src/ray-tracing-demo-cpu/*.cc")
  add_files("src/ray-tracing-demo-cpu/impl/*/*.cc")
  add_packages("tomlplusplus")
