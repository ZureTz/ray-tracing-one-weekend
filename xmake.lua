set_toolchains("gcc")
add_rules("plugin.compile_commands.autoupdate", {lsp = "clangd"})

-- add_requires("pacman::glib2", {alias = "glib2"})

target("generate-ppm-image-multithread")
  set_kind("binary")
  add_files("src/generate-ppm-image-multithread/*.cc")

target("ray-tracing-demo-cpu")
  set_kind("binary")
  add_files("src/ray-tracing-demo-cpu/*.cc")
  -- add_packages("glib2")