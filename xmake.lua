target("generate-ppm-image-multithread")
  set_kind("binary")
  add_files("src/generate-ppm-image-multithread/*.cc")

target("ray-tracing-demo-cpu")
  set_kind("binary")
  add_files("src/ray-tracing-demo-cpu/*.cc")
