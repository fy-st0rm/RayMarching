#define BASE_IMPLEMENTATION
#include "base.h"

extern Context* ctx;

// :config
#define FPS 60
#define SPEED 150.0f
#define MAX_GEOMETRY 32
#define DOWN_SCALE 2.0f
#define FONT_SIZE 20.0f

#define BUTTON_ENABLE_COLOR  ((v4) { 0.2, 0.2, 0.2, 1 })
#define BUTTON_DISABLE_COLOR ((v4) { 0.5, 0.5, 0.5, 1 })

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

char* geometry_name(GeometryType type);

// :state def
typedef enum {
  CON_CAMERA,
  CON_GEOMETRY,
  CON_SPAWN,
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

  // Event
  b8 mouse_clicked;

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
void state_clear();
void state_add_geometry(GeometryType type);
void state_delete_geometry(i32 id);
void state_upload_uniforms();
void state_upload_geometries();
i32  state_select_geometry();
Geometry* state_get_geometry(i32 id);
void state_handle_control(Event event);
void state_update();
void state_raymarch_pass();
void state_ui_pass();
void state_render();

// :ui def
b8 button(const char* label, Rect rect);
void pannel_spawn_geometry();

// :editor def
void editor_geometry_movement_event(Event event);
void editor_camera_movement_event(Event event);
void editor_camera_movement_update();

// :geometry impl
char* geometry_name(GeometryType type) {
  switch (type) {
    case GEO_SPHERE: return "Sphere";
    case GEO_BOX: return "Box";
    default: panic(0, "Unknown geometry type");
  }
}

// :resource impl
void resource_load() {
  state.shaders[SHADER_RAYMARCH] = shader_new_from_file(
    "shaders/raymarch/vert.glsl",
    "shaders/raymarch/frag.glsl"
  );

  state.font = font_new("font.otf", FONT_SIZE);
}

// :state impl
void state_init() {
  state.window = window_new("Game8", 800, 600, true);
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

void state_delete_geometry(i32 id) {
  for (i32 i = 0; i < state.geometries_count; i++) {
    Geometry* geo = &state.geometries[i];
    if (geo->id == id) {
      state.geometries[i] = state.geometries[state.geometries_count - 1];
      state.geometries_count--;
      break;
    }
  }
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

void state_handle_control(Event event) {
  if (event.type == KEYDOWN) {
    switch (event.e.key) {
      case GLFW_KEY_ESCAPE: {
        if (
          state.current_control == CON_GEOMETRY ||
          state.current_control == CON_SPAWN
        ) {
          state.camera.mouse_enable = true;
          state.current_control = CON_CAMERA;
        }
      } break;

      case GLFW_KEY_DELETE: {
        if (state.current_control == CON_GEOMETRY) {
          state_delete_geometry(state.selected_geo);
          state.selected_geo = GEO_NONE;
          state.current_control = CON_CAMERA;
          state.camera.mouse_enable = true;
        }
      } break;

      case GLFW_KEY_G: {
        if (state.current_control == CON_CAMERA) {
          state.camera.mouse_enable = false;
          state.current_control = CON_SPAWN;
        }
      } break;
    }
  }
  else if (event.type == MOUSE_BUTTON_DOWN) {
    switch (event.e.button) {
      case MOUSE_BUTTON_LEFT: {
        state.mouse_clicked = true;

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
  else if (event.type == MOUSE_BUTTON_UP) {
    switch (event.e.button) {
      case MOUSE_BUTTON_LEFT: {
        state.mouse_clicked = false;
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

void state_update() {
  // Camera control using mouse
  pcamera_handle_mouse(&state.camera, state.window);

  if (state.current_control == CON_CAMERA) {
    editor_camera_movement_update();
  }
}

void state_raymarch_pass() {
  fbo_bind(&state.rendered_frame);
  glViewport(
    0, 0,
    state.rendered_frame.width, state.rendered_frame.height
  );

  imr_clear((v4) { 0, 0, 0, 1 });
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

void state_ui_pass() {
  if (state.current_control == CON_SPAWN) {
    pannel_spawn_geometry();
  }

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

void state_render() {
  state_raymarch_pass();

  glViewport(0, 0, state.window.width, state.window.height);
  imr_begin(&state.imr);
  {
    imr_switch_shader_to_default(&state.imr);
    m4 mvp = ocamera_calc_mvp(&state.ui_camera);
    imr_update_mvp(&state.imr, mvp);

    // Render the raymarched rendered texture
    texture_bind(state.rendered_frame.color_texture);
    imr_push_quad_tex(
      &state.imr,
      (v3) { 0, 0, 0 },
      (v2) { state.window.width, state.window.height },
      (Rect) { 0, 1, 1, -1 },
      state.rendered_frame.color_texture.id,
      m4_identity(),
      (v4) { 1, 1, 1, 1 }
    );

    // Render the overlay ui
    state_ui_pass();
  }
  imr_end(&state.imr);
}

// :ui impl
b8 button(const char* label, Rect rect) {
  v2 mouse = event_mouse_pos(state.window);

  v4 color = BUTTON_DISABLE_COLOR;
  if (point_in_rect(mouse, rect)) {
    color = BUTTON_ENABLE_COLOR;
  }

  imr_push_quad(
    &state.imr,
    (v3) { rect.x, rect.y, 0 },
    (v2) { rect.w, rect.h },
    m4_identity(),
    color
  );

  v2 label_size = font_calc_size(&state.font, label);
  font_render(
    &state.imr,
    &state.font,
    label,
    (v3) {
      rect.x + rect.w / 2 - label_size.x / 2,
      rect.y + rect.h / 2 + label_size.y / 2
    },
    (v4) { 1, 1, 1, 1 }
  );

  return (state.mouse_clicked && point_in_rect(mouse, rect));
}

void pannel_spawn_geometry() {
  f32 gap = 10.0f;
  v2 pannel_size = { 200, 300 };
  v3 pannel_pos = {
    state.window.width - pannel_size.x - gap,
    state.window.height - pannel_size.y - gap,
    1
  };

  // Render pannel
  imr_push_quad(
    &state.imr,
    pannel_pos,
    pannel_size,
    m4_identity(),
    (v4) { 0.5, 0.5, 0.5, 0.5 }
  );

  // Render label
  const char* label = "Geometries";
  v2 label_size = font_calc_size(&state.font, label);
  v3 label_pos = {
    pannel_pos.x + pannel_size.x / 2 - label_size.x / 2,
    pannel_pos.y + label_size.y + gap,
    1
  };
  font_render(
    &state.imr,
    &state.font,
    label,
    label_pos,
    (v4) { 1, 1, 1, 1 }
  );

  // Render buttons
  f32 button_gap = 15.0f;
  v2 button_size = { 0.8 * pannel_size.x, 50 };
  f32 x = pannel_pos.x + pannel_size.x / 2 - button_size.x / 2;

  for (i32 i = 0; i < GEO_TOTAL; i++) {
    if (i == GEO_NONE) continue;

    f32 y = label_pos.y + button_gap + (i - 1) * (button_size.y + button_gap);
    if (button(
      geometry_name(i),
      (Rect) { x, y, button_size.x, button_size.y }
    )) {
      state_add_geometry(i);
    }
  }
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

    state_update();
    state_render();

    frame_controller_end(&state.fc);
    window_update(&state.window);

    // Reset the mouse clicked at the end of frame
    state.mouse_clicked = false;
  }

  state_clean();
  return 0;
}
