#define BASE_IMPLEMENTATION
#include "base.h"

extern Context* ctx;

// :config
#define FPS 60
#define SPEED 150.0f
#define MAX_GEOMETRY 32
#define DOWN_SCALE 2.0f

char* controls_text = 
  "WASD    - Movement\n"
  "L-SHIFT - Go Down\n"
  "SPACE   - Go Up\n"
  "ESC     - Toggle Mouse\n"
  "h       - Close help";

// :common def
typedef enum {
  LEFT,
  RIGHT,
  UP,
  DOWN,
  FRONT,
  BACK,
  TOTAL_DIR,
} Dir;

// :resources def
typedef enum {
  SHADER_RAYMARCH,
  TOTAL_SHADERS,
} Shaders;

void resource_load();

// :geometry def
typedef enum {
  GEO_NONE,
  GEO_SPHERE,
  GEO_BOX,
  GEO_TOTAL,
} GeometryType;

typedef struct {
  GeometryType type;
  i32 id;
  v3 color;
  v3 center;

  union {
    struct {
      f32 radius;
    } sphere;

    struct {
      v3 half_size;
    } box;
  };
} Geometry;

// :state def
typedef enum {
  CON_MOUSE,
  CON_CAMERA,
  CON_GEOMETRY,
} ControlType;

typedef struct {
  Window window;
  IMR imr;
  FrameController fc;
  PCamera camera;
  OCamera ui_camera;
  FBO rendered_frame;

  Font font;
  b8 help;

  // Shaders
  Shader shaders[TOTAL_SHADERS];
  i32 shader_count;

  // Movement
  b8 movement[TOTAL_DIR];

  // Scene
  Geometry geometries[MAX_GEOMETRY];
  i32 geometries_count;

  // Control
  ControlType current_control;
  i32 selected_geo;
} State;

State state = {0}; // Global state variable

void state_init();
void state_clean();
void state_handle_control(Event event);
void state_update_program();
void state_clear();
void state_add_geometry(GeometryType type);
void state_upload_uniforms();
void state_upload_geometries();
i32  state_select_geometry();
Geometry* state_get_geometry(i32 id);

// :editor def
void editor_geometry_movement_event(Event event);
void editor_camera_movement_event(Event event);
void editor_camera_movement_update();

// :resource impl
void resource_load() {
  state.shaders[SHADER_RAYMARCH] = shader_new_from_file(
    "shaders/raymarch/vert.glsl",
    "shaders/raymarch/frag.glsl"
  );

  state.font = font_new("font.otf", 16);
}

// :state impl
void state_init() {
  state.window = window_new("Game8", 800, 600, false);
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

  state.rendered_frame = fbo_new(
    state.window.width / DOWN_SCALE,
    state.window.height / DOWN_SCALE
  );

  state.help = false;
  state.current_control = CON_CAMERA;
  state.camera.mouse_enable = true;

  resource_load();
}

void state_clean() {
  fbo_delete(&state.rendered_frame);
  font_delete(&state.font);
  imr_delete(&state.imr);
  window_delete(state.window);
}

void state_handle_control(Event event) {
  if (event.type == KEYDOWN) {
    switch (event.e.key) {
      case GLFW_KEY_ESCAPE: {
        if (state.current_control == CON_GEOMETRY) {
          state.camera.mouse_enable = true;
          state.current_control = CON_CAMERA;
        }
      } break;
    }
  }
  else if (event.type == MOUSE_BUTTON_DOWN) {
    switch (event.e.button) {
      case MOUSE_BUTTON_LEFT: {
        if (state.current_control == CON_CAMERA) {
          i32 geo = state_select_geometry();
          if (geo > 0) {
            state.current_control = CON_GEOMETRY;
            state.selected_geo = geo;
            state.camera.mouse_enable = false;
          }
        }
      } break;
    }
  }

  switch (state.current_control) {
    case CON_CAMERA:
      editor_camera_movement_event(event);
      break;
    case CON_GEOMETRY:
      editor_geometry_movement_event(event);
      break;
  }
}

void state_update_program() {
  // Camera control using mouse
  pcamera_handle_mouse(&state.camera, state.window);

  if (state.current_control == CON_CAMERA) {
    editor_camera_movement_update();
  }
}

void state_clear() {
  state.geometries_count = 0;
}

void state_add_geometry(GeometryType type) {
  Geometry geo = {
    .type = type,
    .id = state.geometries_count + 1,
    .color = (v3) { 1, 1, 1 },
    .center = (v3) { 0, 0, 0 },
  };

  switch (type) {
    case GEO_SPHERE: {
      geo.sphere.radius = 1.0f;
    } break;

    case GEO_BOX: {
      geo.box.half_size = (v3) { 0.5, 0.5, 0.5 };
    } break;
  }

  state.geometries[state.geometries_count++] = geo;
}

void state_upload_uniforms() {
  {
    int loc = GLCall(glGetUniformLocation(state.shaders[SHADER_RAYMARCH], "u_resolution"));
    GLCall(glUniform2f(loc, (f32) state.rendered_frame.width, (f32) state.rendered_frame.height));
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

void state_upload_geometries() {
  Shader shader = state.shaders[SHADER_RAYMARCH];
  GLint loc = 0;

  for (i32 i = 0; i < state.geometries_count; i++) {
    Geometry geo = state.geometries[i];

    char uniform[64];

    snprintf(uniform, sizeof(uniform), "geometries[%d].type", i);
    loc = GLCall(glGetUniformLocation(shader, uniform));
    GLCall(glUniform1i(
      loc,
      geo.type
    ));

    snprintf(uniform, sizeof(uniform), "geometries[%d].id", i);
    loc = GLCall(glGetUniformLocation(shader, uniform));
    GLCall(glUniform1i(
      loc,
      geo.id
    ));

    snprintf(uniform, sizeof(uniform), "geometries[%d].center", i);
    loc = GLCall(glGetUniformLocation(shader, uniform));
    GLCall(glUniform3f(
      loc,
      geo.center.x,
      geo.center.y,
      geo.center.z
    ));

    snprintf(uniform, sizeof(uniform), "geometries[%d].color", i);
    loc = GLCall(glGetUniformLocation(shader, uniform));
    GLCall(glUniform3f(
      loc,
      geo.color.x,
      geo.color.y,
      geo.color.z
    ));

    switch (geo.type) {
      case GEO_SPHERE: {
        snprintf(uniform, sizeof(uniform), "geometries[%d].data", i);
        loc = GLCall(glGetUniformLocation(shader, uniform));
        GLCall(glUniform4f(
          loc,
          geo.sphere.radius,
          0,
          0,
          0
        ));
      } break;

      case GEO_BOX: {
        snprintf(uniform, sizeof(uniform), "geometries[%d].data", i);
        loc = GLCall(glGetUniformLocation(shader, uniform));
        GLCall(glUniform4f(
          loc,
          geo.box.half_size.x,
          geo.box.half_size.y,
          geo.box.half_size.z,
          0
        ));
      } break;
    }

  }

  loc = GLCall(glGetUniformLocation(shader, "geometries_count"));
  GLCall(glUniform1i(loc, state.geometries_count));
}

i32 state_select_geometry() {
  // v2 mouse = event_mouse_pos(state.window);
  // mouse.y = state.window.height - mouse.y;
  // mouse.x = mouse.x / state.window.width * state.rendered_frame.width;
  // mouse.y = mouse.y / state.window.height * state.rendered_frame.height;

  GLubyte pixel[4];
  glBindFramebuffer(GL_READ_FRAMEBUFFER, state.rendered_frame.id);
  glReadBuffer(GL_COLOR_ATTACHMENT1);

  glReadPixels(
    state.rendered_frame.width / 2.0f,
    state.rendered_frame.height / 2.0f,
    1,
    1,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    pixel
  );

  return pixel[0];
}

Geometry* state_get_geometry(i32 id) {
  for (i32 i = 0; i < state.geometries_count; i++) {
    Geometry* geo = &state.geometries[i];
    if (geo->id == id) return geo;
  }
  return NULL;
}

// :editor impl
void editor_geometry_movement_event(Event event) {
  panic(state.selected_geo > 0, "Selected Geometry ID is 0");

  Geometry* geo = state_get_geometry(state.selected_geo);
  panic(geo, "Selected Geometry is NULL");

  if (event.type == KEYDOWN) {
    switch (event.e.key) {
      case GLFW_KEY_W:
        geo->center.z += SPEED * state.fc.dt;
        break;
      case GLFW_KEY_A:
        geo->center.x += SPEED * state.fc.dt;
        break;
      case GLFW_KEY_S:
        geo->center.z -= SPEED * state.fc.dt;
        break;
      case GLFW_KEY_D:
        geo->center.x -= SPEED * state.fc.dt;
        break;
      case GLFW_KEY_SPACE:
        geo->center.y += SPEED * state.fc.dt;
        break;
      case GLFW_KEY_LEFT_SHIFT:
        geo->center.y -= SPEED * state.fc.dt;
        break;
    }
  }
}

void editor_camera_movement_event(Event event) {
  if (event.type == KEYDOWN) {
    switch (event.e.key) {
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

void editor_camera_movement_update() {
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

  state_clear();
  state_add_geometry(GEO_SPHERE);
  state_add_geometry(GEO_BOX);

  while (!state.window.should_close) {
    frame_controller_start(&state.fc);

    // Event handling
    Event event = {0};
    while (event_poll(state.window, &event)) {
      state_handle_control(event);
      if (event.type == KEYDOWN) {
        switch (event.e.key) {
          case GLFW_KEY_H: {
            state.help = (state.help) ? false : true;
          } break;
        }
      }
    }

    state_update_program();

    // Rendering pass
    {
      fbo_bind(&state.rendered_frame);
      glViewport(
        0, 0,
        state.rendered_frame.width, state.rendered_frame.height
      );

      imr_clear((v4) { 0, 0, 0, 1.0f });
      imr_begin(&state.imr);
      {

        imr_switch_shader(&state.imr, state.shaders[SHADER_RAYMARCH]);
        state_upload_uniforms();
        state_upload_geometries();

        imr_push_quad(
          &state.imr,
          (v3) {-1, -1, 0},
          (v2) {2, 2},
          m4_identity(),
          (v4) {0, 0, 0, 1}
        );
      }
      imr_end(&state.imr);

      fbo_unbind();
    }

    // Final Rendering Pass
    {
      glViewport(0, 0, state.window.width, state.window.height);

      imr_begin(&state.imr);
      {
        imr_switch_shader_to_default(&state.imr);

        m4 mvp = ocamera_calc_mvp(&state.ui_camera);
        imr_update_mvp(&state.imr, mvp);

        texture_bind(state.rendered_frame.color_texture);

        // Render the frame
        imr_push_quad_tex(
          &state.imr,
          (v3) { 0, 0, 0 },
          (v2) { state.window.width, state.window.height },
          (Rect) { 0, 1, 1, -1 },
          state.rendered_frame.color_texture.id,
          m4_identity(),
          (v4) { 1, 1, 1, 1 }
        );

        // UI

        // Render crosshair
        v2 crosshair_size = { 3, 3 };
        imr_push_quad(
          &state.imr,
          (v3) {
            state.window.width / 2.0f - crosshair_size.x / 2.0f,
            state.window.height / 2.0f - crosshair_size.y / 2.0f,
            0
          },
          crosshair_size,
          m4_identity(),
          (v4) { 0, 1, 0, 1 }
        );

        // Render FPS
        char text[100];
        snprintf(text, sizeof(text), "FFS: %d", state.fc.fps);
        font_render(
          &state.imr, &state.font, text,
          (v3) { 30, 30, 0 },
          (v4) { 1, 1, 1, 1 }
        );

        // Render Controls
        if (state.help) {
          font_render(
            &state.imr, &state.font, controls_text,
            (v3) { state.window.width - 200, 30, 0 },
            (v4) { 1, 1, 1, 1 }
          );
        } else {
          font_render(
            &state.imr, &state.font, "Press [h] for Help",
            (v3) { state.window.width - 200, 30, 0 },
            (v4) { 1, 1, 1, 1 }
          );
        }
      }
      imr_end(&state.imr);
    }

    frame_controller_end(&state.fc);
    window_update(&state.window);
  }

  state_clean();
  return 0;
}
