#define CBUILD_IMPLEMENTATION
#include "cbuild.h"

int main(int argc, char** argv) {
  cbuild_rebuild(argc, argv);

  CBuild build("gcc");
  build
    .out("bin", "game")
    .flags({
      "-D_GNU_SOURCE",
    })
    .inc_paths({
      "./src/",
      "./src/external/glew/include/",
      "./src/external/glfw/include/",
    })
    .libs({
      "GL",
      "GLU",
      "m",
    })
    .src({
      "src/external/glew/src/glew.c",
      "src/external/glfw/src/glfw.c",
      "src/main.c",
    })
    .build()
    .run()
    .clean();

  return 0;
}
