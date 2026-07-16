#define BASE_IMPLEMENTATION
#include "base.h"

extern Context* ctx;

// :config
#define FPS 60

// :assets def
typedef enum {
  SHADER_RAYMARCH,
  TOTAL_SHADERS,
} Shaders;

// :state def
typedef struct {
  Window window;
  IMR imr;
  FrameController fc;
  OCamera camera;

  // Shaders
  Shader shaders[TOTAL_SHADERS];
  i32 shader_count;
} State;

State state = {0}; // Global state variable

void state_init();
void state_clean();
void load_shaders();

// :state impl
void state_init() {
  state.window = window_new("Game8", 800, 800, true);
  state.imr = imr_new();
  state.fc = frame_controller_new(FPS);
  state.camera = ocamera_new(
    (v2) {0,0},
    1,
    (OCamera_Boundary) {
      .left = 0,
      .right = state.window.width,
      .top = 0,
      .bottom = state.window.height,
      .near = -1.0f,
      .far = 1000.0f,
    }
  );

  load_shaders();
}

void state_clean() {
  window_delete(state.window);
  imr_delete(&state.imr);
}

void load_shaders() {
  state.shaders[SHADER_RAYMARCH] = shader_new_from_file(
    "shaders/raymarch/vert.glsl",
    "shaders/raymarch/frag.glsl"
  );
}

// :main
int main() {
  state_init();
  log_info("Opengl Version: %s\n", glGetString(GL_VERSION));

  while (!state.window.should_close) {
    frame_controller_start(&state.fc);

    // Event handling
    Event event = {0};
    while (event_poll(state.window, &event)) {
    }

    // Rendering pass
    {
      glViewport(0, 0, state.window.width, state.window.height);

      m4 mvp = ocamera_calc_mvp(&state.camera);
      imr_update_mvp(&state.imr, mvp);

      imr_clear((v4) { .5f, .5f, .5f, 1.0f });
      imr_begin(&state.imr);
      {
        imr_switch_shader(&state.imr, state.shaders[SHADER_RAYMARCH]);


        int loc = glGetUniformLocation(state.shaders[SHADER_RAYMARCH], "u_resolution");
        glUniform2f(loc, (f32) state.window.width, (f32) state.window.height);

        imr_push_quad(
          &state.imr,
          (v3) {-1, -1, 0},
          (v2) {2, 2},
          m4_identity(),
          (v4) {0, 0, 0, 1}
        );
      }
      imr_end(&state.imr);
    }

    frame_controller_end(&state.fc);
    window_update(&state.window);
  }

  state_clean();
  return 0;
}
