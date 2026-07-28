#define BASE_IMPLEMENTATION
#include "base.h"

extern Context* ctx;

// :config
#define FPS 60
#define SPEED 250.0f
#define MAX_GEOMETRY 32

char* controls_text = 
  "WASD    - Movement\n"
  "L-SHIFT - Go Down\n"
  "SPACE   - Go Up\n"
  "ESC     - Toggle Mouse\n";

// :assets def
typedef enum {
  SHADER_RAYMARCH,
  TOTAL_SHADERS,
} Shaders;

// :geometry def
typedef struct {
  v3 center;
  f32 radius;
  v3 color;
} Sphere;

typedef struct {
  v3 center;
  v3 half_size;
  v3 color;
} Box;

// :state def
typedef enum {
  LEFT,
  RIGHT,
  UP,
  DOWN,
  FRONT,
  BACK,
  TOTAL_DIR,
} Dir;

typedef struct {
  Window window;
  IMR imr;
  FrameController fc;
  PCamera camera;
  OCamera ui_camera;

  Font font;

  // Shaders
  Shader shaders[TOTAL_SHADERS];
  i32 shader_count;

  // Movement
  b8 movement[TOTAL_DIR];

  // Scene
  Sphere spheres[MAX_GEOMETRY];
  i32 spheres_count;

  Box boxes[MAX_GEOMETRY];
  i32 boxes_count;
} State;

State state = {0}; // Global state variable

void state_init();
void state_clean();
void load_shaders();
void upload_uniforms();

void add_sphere(Sphere sphere);
void add_box(Box box);
void upload_sphere();
void upload_box();
void upload_geometries();

// :movement def
void movement_event(Event event);
void movement_update();

// :state impl
void state_init() {
  state.window = window_new("Game8", 800, 800, true);
  state.imr = imr_new();
  state.fc = frame_controller_new(FPS);
  state.camera = pcamera_new(
    (v3) {0,0,-3},
    (v3) {0,0,1},
    0.1f,
    (PCamera_Info) {
      .aspect_ratio = (f32) state.window.width / state.window.height,
      .fov = 45.0f,
      .near = 0.0f,
      .far = 1000.0f,
    }
  );
  state.ui_camera = ocamera_new(
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

  state.font = font_new("font.otf", 32);

  load_shaders();
}

void state_clean() {
  font_delete(&state.font);
  imr_delete(&state.imr);
  window_delete(state.window);
}

void load_shaders() {
  state.shaders[SHADER_RAYMARCH] = shader_new_from_file(
    "shaders/raymarch/vert.glsl",
    "shaders/raymarch/frag.glsl"
  );
}

void upload_uniforms() {
  {
    int loc = GLCall(glGetUniformLocation(state.shaders[SHADER_RAYMARCH], "u_resolution"));
    GLCall(glUniform2f(loc, (f32) state.window.width, (f32) state.window.height));
  }

  {
    int loc = GLCall(glGetUniformLocation(state.shaders[SHADER_RAYMARCH], "u_camera_pos"));
    GLCall(glUniform3f(loc, state.camera.pos.x, state.camera.pos.y, state.camera.pos.z));
  }

  {
    int loc = GLCall(glGetUniformLocation(state.shaders[SHADER_RAYMARCH], "u_camera_forward"));
    GLCall(glUniform3f(loc, state.camera.forward.x, state.camera.forward.y, state.camera.forward.z));
  }

  {
    int loc = GLCall(glGetUniformLocation(state.shaders[SHADER_RAYMARCH], "u_camera_right"));
    GLCall(glUniform3f(loc, state.camera.right.x, state.camera.right.y, state.camera.right.z));
  }

  {
    int loc = GLCall(glGetUniformLocation(state.shaders[SHADER_RAYMARCH], "u_camera_up"));
    GLCall(glUniform3f(loc, state.camera.up.x, state.camera.up.y, state.camera.up.z));
  }
}

void add_sphere(Sphere sphere) {
  panic(
    state.spheres_count + 1 < MAX_GEOMETRY,
    "Cannot fit any more spheres."
  );

  state.spheres[state.spheres_count++] = sphere;
}

void add_box(Box box) {
  panic(
    state.boxes_count + 1 < MAX_GEOMETRY,
    "Cannot fit any more boxes."
  );

  state.boxes[state.boxes_count++] = box;
}

void upload_sphere() {
  Shader shader = state.shaders[SHADER_RAYMARCH];

  for (i32 i = 0; i < state.spheres_count; i++) {
    Sphere sphere = state.spheres[i];

    char uniform[64];

    snprintf(uniform, sizeof(uniform), "spheres[%d].center", i);
    GLint loc = GLCall(glGetUniformLocation(shader, uniform));
    GLCall(glUniform3f(
      loc,
      sphere.center.x,
      sphere.center.y,
      sphere.center.z
    ));

    snprintf(uniform, sizeof(uniform), "spheres[%d].radius", i);
    loc = GLCall(glGetUniformLocation(shader, uniform));
    GLCall(glUniform1f(
      loc,
      sphere.radius
    ));

    snprintf(uniform, sizeof(uniform), "spheres[%d].color", i);
    loc = GLCall(glGetUniformLocation(shader, uniform));
    GLCall(glUniform3f(
      loc,
      sphere.color.x,
      sphere.color.y,
      sphere.color.z
    ));
  }

  GLint loc = GLCall(glGetUniformLocation(shader, "spheres_count"));
  GLCall(glUniform1i(loc, state.spheres_count));
}

void upload_box() {
  Shader shader = state.shaders[SHADER_RAYMARCH];

  for (i32 i = 0; i < state.boxes_count; i++) {
    Box box = state.boxes[i];

    char uniform[64];

    snprintf(uniform, sizeof(uniform), "boxes[%d].center", i);
    GLint loc = GLCall(glGetUniformLocation(shader, uniform));
    GLCall(glUniform3f(
      loc,
      box.center.x,
      box.center.y,
      box.center.z
    ));

    snprintf(uniform, sizeof(uniform), "boxes[%d].half_size", i);
    loc = GLCall(glGetUniformLocation(shader, uniform));
    GLCall(glUniform3f(
      loc,
      box.half_size.x,
      box.half_size.y,
      box.half_size.z
    ));

    snprintf(uniform, sizeof(uniform), "boxes[%d].color", i);
    loc = GLCall(glGetUniformLocation(shader, uniform));
    GLCall(glUniform3f(
      loc,
      box.color.x,
      box.color.y,
      box.color.z
    ));
  }

  GLint loc = GLCall(glGetUniformLocation(shader, "boxes_count"));
  GLCall(glUniform1i(loc, state.boxes_count));
}

void upload_geometries() {
  upload_sphere();
  upload_box();
}


// :movement impl
void movement_event(Event event) {
  if (event.type == KEYDOWN) {
    switch (event.e.key) {
      case GLFW_KEY_ESCAPE:
        state.camera.mouse_enable = state.camera.mouse_enable ? false : true;
        break;
      case GLFW_KEY_W:
        state.movement[FRONT] = true;
        break;
      case GLFW_KEY_A:
        state.movement[LEFT] = true;
        break;
      case GLFW_KEY_S:
        state.movement[BACK] = true;
        break;
      case GLFW_KEY_D:
        state.movement[RIGHT] = true;
        break;
      case GLFW_KEY_SPACE:
        state.movement[UP] = true;
        break;
      case GLFW_KEY_LEFT_SHIFT:
        state.movement[DOWN] = true;
        break;
    }
  }
  else if (event.type == KEYUP) {
    switch (event.e.key) {
      case GLFW_KEY_W:
        state.movement[FRONT] = false;
        break;
      case GLFW_KEY_A:
        state.movement[LEFT] = false;
        break;
      case GLFW_KEY_S:
        state.movement[BACK] = false;
        break;
      case GLFW_KEY_D:
        state.movement[RIGHT] = false;
        break;
      case GLFW_KEY_SPACE:
        state.movement[UP] = false;
        break;
      case GLFW_KEY_LEFT_SHIFT:
        state.movement[DOWN] = false;
        break;
    }
  }
}

void movement_update() {
  if (state.movement[FRONT]) {
    pcamera_change_pos(&state.camera, v3_mul_scalar(state.camera.forward, SPEED * state.fc.dt));
  }
  if (state.movement[BACK]) {
    pcamera_change_pos(&state.camera, v3_mul_scalar(state.camera.forward, -SPEED * state.fc.dt));
  }
  if (state.movement[LEFT]) {
    pcamera_change_pos(&state.camera, v3_mul_scalar(state.camera.right, -SPEED * state.fc.dt));
  }
  if (state.movement[RIGHT]) {
    pcamera_change_pos(&state.camera, v3_mul_scalar(state.camera.right, SPEED * state.fc.dt));
  }
  if (state.movement[UP]) {
    pcamera_change_pos(&state.camera, v3_mul_scalar(state.camera.up, SPEED * state.fc.dt));
  }
  if (state.movement[DOWN]) {
    pcamera_change_pos(&state.camera, v3_mul_scalar(state.camera.up, -SPEED * state.fc.dt));
  }
}

// :main
int main() {
  state_init();
  log_info("Opengl Version: %s\n", glGetString(GL_VERSION));

  add_sphere((Sphere) {
    .center = (v3) {4.5,3,-1},
    .radius = 1.0f,
    .color = (v3) {1, 0, 0},
  });

  add_sphere((Sphere) {
    .center = (v3) {3,0,0},
    .radius = 1.0f,
    .color = (v3) {1, 1, 0},
  });

  add_box((Box) {
    .center = (v3) { 4, 0, 0 },
    .half_size = (v3) { 1, 1, 1 },
    .color = (v3) { 0, 0, 1 },
  });

  add_box((Box) {
    .center = (v3) { 0, -2, 0 },
    .half_size = (v3) { 50, 0.5, 50 },
    .color = (v3) { 1, 1, 1 },
  });

  while (!state.window.should_close) {
    frame_controller_start(&state.fc);

    // Event handling
    Event event = {0};
    while (event_poll(state.window, &event)) {
      movement_event(event);
    }

    // Camera movement
    movement_update();

    // Rendering pass
    {
      glViewport(0, 0, state.window.width, state.window.height);

      // Camera mouse control
      pcamera_handle_mouse(&state.camera, state.window);

      imr_clear((v4) { .5f, .5f, .5f, 1.0f });
      imr_begin(&state.imr);
      {
        imr_switch_shader(&state.imr, state.shaders[SHADER_RAYMARCH]);

        upload_uniforms();
        upload_geometries();

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

    // UI Rendering Pass
    {
      glViewport(0, 0, state.window.width, state.window.height);

      imr_begin(&state.imr);
      {
        imr_switch_shader_to_default(&state.imr);

        m4 mvp = ocamera_calc_mvp(&state.ui_camera);
        imr_update_mvp(&state.imr, mvp);

        // Render FPS
        char text[100];
        snprintf(text, sizeof(text), "FFS: %d", state.fc.fps);
        font_render(
          &state.imr, &state.font, text,
          (v3) { 50, 50, 0 },
          (v4) { 1, 1, 1, 1 }
        );

        // Render Controls
        font_render(
          &state.imr, &state.font, controls_text,
          (v3) { state.window.width - 400, 50, 0 },
          (v4) { 1, 1, 1, 1 }
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
