#define BASE_IMPLEMENTATION
#include "base.h"

extern Context* ctx;

// :config
#define FPS 60
#define MAX_GEOMETRY 32
#define DOWN_SCALE 3.0f
#define FONT_SIZE 20.0f

#define SPEED 10.0f
#define GEOMETRY_SCALE_SPEED 0.1f;
#define SMOOTHNESS_SPEED 0.01f;

#define BUTTON_ENABLE_COLOR  ((v4) { 0.2, 0.2, 0.2, 1 })
#define BUTTON_DISABLE_COLOR ((v4) { 0.5, 0.5, 0.5, 1 })

#define SPAWN_GEOMETRY_PANNEL_SIZE ((v2) { 200, 300 })
#define GEOMETRY_DATA_PANNEL_SIZE  ((v2) { 500, 200 })
#define GEOMETRY_HELP_PANNEL_SIZE  ((v2) { 400, 200 })
#define GEOMETRY_COLOR_PANNEL_SIZE ((v2) { 400, 200 })
#define SETTINGS_PANNEL_SIZE       ((v2) { 350, 300 })

#define PALETTE_SIZE 64
static v4 PALETTE[PALETTE_SIZE] = {
  // Grayscale
  {0.000f, 0.000f, 0.000f, 1.0f},
  {0.133f, 0.125f, 0.204f, 1.0f},
  {0.263f, 0.263f, 0.310f, 1.0f},
  {0.412f, 0.416f, 0.416f, 1.0f},
  {0.545f, 0.549f, 0.561f, 1.0f},
  {0.710f, 0.710f, 0.710f, 1.0f},
  {0.851f, 0.851f, 0.851f, 1.0f},
  {1.000f, 1.000f, 1.000f, 1.0f},

  // Reds
  {0.247f, 0.000f, 0.000f, 1.0f},
  {0.498f, 0.000f, 0.000f, 1.0f},
  {0.749f, 0.000f, 0.000f, 1.0f},
  {1.000f, 0.000f, 0.000f, 1.0f},
  {1.000f, 0.333f, 0.333f, 1.0f},
  {1.000f, 0.533f, 0.533f, 1.0f},
  {1.000f, 0.733f, 0.733f, 1.0f},
  {1.000f, 0.878f, 0.878f, 1.0f},

  // Oranges / Browns
  {0.290f, 0.165f, 0.000f, 1.0f},
  {0.478f, 0.247f, 0.000f, 1.0f},
  {0.651f, 0.353f, 0.000f, 1.0f},
  {0.851f, 0.478f, 0.000f, 1.0f},
  {1.000f, 0.647f, 0.000f, 1.0f},
  {1.000f, 0.749f, 0.400f, 1.0f},
  {1.000f, 0.847f, 0.659f, 1.0f},
  {1.000f, 0.941f, 0.839f, 1.0f},

  // Yellows
  {0.302f, 0.302f, 0.000f, 1.0f},
  {0.502f, 0.502f, 0.000f, 1.0f},
  {0.702f, 0.702f, 0.000f, 1.0f},
  {1.000f, 1.000f, 0.000f, 1.0f},
  {1.000f, 1.000f, 0.400f, 1.0f},
  {1.000f, 1.000f, 0.600f, 1.0f},
  {1.000f, 1.000f, 0.800f, 1.0f},
  {1.000f, 1.000f, 0.941f, 1.0f},

  // Greens
  {0.000f, 0.200f, 0.000f, 1.0f},
  {0.000f, 0.400f, 0.000f, 1.0f},
  {0.000f, 0.600f, 0.200f, 1.0f},
  {0.000f, 0.800f, 0.000f, 1.0f},
  {0.000f, 1.000f, 0.000f, 1.0f},
  {0.400f, 1.000f, 0.400f, 1.0f},
  {0.667f, 1.000f, 0.667f, 1.0f},
  {0.878f, 1.000f, 0.878f, 1.0f},

  // Cyans
  {0.000f, 0.200f, 0.200f, 1.0f},
  {0.000f, 0.400f, 0.400f, 1.0f},
  {0.000f, 0.600f, 0.600f, 1.0f},
  {0.000f, 0.800f, 0.800f, 1.0f},
  {0.000f, 1.000f, 1.000f, 1.0f},
  {0.400f, 1.000f, 1.000f, 1.0f},
  {0.667f, 1.000f, 1.000f, 1.0f},
  {0.878f, 1.000f, 1.000f, 1.0f},

  // Blues
  {0.000f, 0.122f, 0.302f, 1.0f},
  {0.000f, 0.247f, 0.498f, 1.0f},
  {0.000f, 0.333f, 0.800f, 1.0f},
  {0.000f, 0.502f, 1.000f, 1.0f},
  {0.200f, 0.600f, 1.000f, 1.0f},
  {0.400f, 0.698f, 1.000f, 1.0f},
  {0.600f, 0.800f, 1.000f, 1.0f},
  {0.867f, 0.933f, 1.000f, 1.0f},

  // Purples / Magentas
  {0.200f, 0.000f, 0.200f, 1.0f},
  {0.400f, 0.000f, 0.400f, 1.0f},
  {0.600f, 0.000f, 0.600f, 1.0f},
  {0.800f, 0.000f, 0.800f, 1.0f},
  {1.000f, 0.000f, 1.000f, 1.0f},
  {1.000f, 0.400f, 1.000f, 1.0f},
  {1.000f, 0.667f, 1.000f, 1.0f},
  {1.000f, 0.878f, 1.000f, 1.0f},
};

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
  SHADER_UPSCALE,
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
  CON_SETTINGS,
  CON_TOTAL,
} ControlType;

char* control_type_name(ControlType type);

typedef enum {
  OP_UNION,
  OP_INTERSECTION,
  OP_TOTAL,
} RenderOp;

char* render_op_name(RenderOp op);

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
  b8 rshift_hold;

  // Shaders
  Shader shaders[TOTAL_SHADERS];
  i32 shader_count;

  // Movement
  b8 movement[TOTAL_DIR];
  b8 size[TOTAL_DIR];

  // Scene
  Geometry geometries[MAX_GEOMETRY];
  i32 geometries_count;

  // Control
  ControlType current_control;
  i32 selected_geo_id;
  Geometry* selected_geo;
  v4 selected_color;

  // Rendering settings
  RenderOp render_op;
  f32 op_smoothness;
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
void state_upscale_pass();
void state_render();

// :ui def
b8 button(const char* label, Rect rect);
b8 color_selector(Rect rect, v4* selected_color);
void pannel_spawn_geometry();
void pannel_geometry_data();
void pannel_geometry_help();
void pannel_geometry_color();
void pannel_settings();

// :editor def
void editor_movement_event(Event event);
void editor_camera_movement_update();
void editor_geometry_movement_update();
void editor_settings_update();

// :geometry impl
char* geometry_name(GeometryType type) {
  STATIC_ASSERT(GEO_TOTAL == 3, "Update here!");
  switch (type) {
    case GEO_SPHERE: return "Sphere";
    case GEO_BOX: return "Box";
    default: panic(0, "Unknown geometry type");
  }
}

// :resource impl
void resource_load() {
  STATIC_ASSERT(TOTAL_SHADERS == 2, "Update here!");
  state.shaders[SHADER_RAYMARCH] = shader_new_from_file(
    "shaders/raymarch/vert.glsl",
    "shaders/raymarch/frag.glsl"
  );
  state.shaders[SHADER_UPSCALE] = shader_new_from_file(
    "shaders/upscale/vert.glsl",
    "shaders/upscale/frag.glsl"
  );

  state.font = font_new("font.otf", FONT_SIZE);
}

// :state impl
char* control_type_name(ControlType type) {
  STATIC_ASSERT(CON_TOTAL == 4, "Update here!");
  switch (type) {
    case CON_CAMERA: return "FPS Camera";
    case CON_GEOMETRY: return "Edit Geometry";
    case CON_SPAWN: return "Spawn Geometry";
    case CON_SETTINGS: return "Settings";
    default: panic(0, "Unexpected type: %d", type);
  }
}

char* render_op_name(RenderOp op) {
  STATIC_ASSERT(OP_TOTAL == 2, "Update here!");
  switch (op) {
    case OP_UNION: return "Union";
    case OP_INTERSECTION: return "Intersection";
    default: panic(0, "Unexpected op: %d", op);
  }
}

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
  state.selected_color = (v4) {1, 1, 1, 1};

  state.render_op = OP_UNION;
  state.op_smoothness = 0;

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

  STATIC_ASSERT(GEO_TOTAL == 3, "Update here!");
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

  loc = GLCall(glGetUniformLocation(shader, "render_op"));
  GLCall(glUniform1i(loc, state.render_op));

  loc = GLCall(glGetUniformLocation(shader, "op_smoothness"));
  GLCall(glUniform1f(loc, state.op_smoothness));

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

    STATIC_ASSERT(GEO_TOTAL == 3, "Update here!");
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
          state.current_control == CON_SPAWN    ||
          state.current_control == CON_SETTINGS
        ) {
          state.camera.mouse_enable = true;
          state.current_control = CON_CAMERA;
          state.selected_geo_id = GEO_NONE;
          state.selected_geo = NULL;
          state.selected_color = (v4){1};
        }
      } break;

      case GLFW_KEY_RIGHT_SHIFT: {
        state.rshift_hold = true;
      } break;

      case GLFW_KEY_C: {
        state.camera.mouse_enable = false;
        state.current_control = CON_SETTINGS;
      } break;

      case GLFW_KEY_DELETE: {
        if (state.current_control == CON_GEOMETRY) {
          state_delete_geometry(state.selected_geo_id);
          state.selected_geo_id = GEO_NONE;
          state.selected_geo = NULL;
          state.current_control = CON_CAMERA;
          state.camera.mouse_enable = true;
        }
      } break;
    }
  }
  else if (event.type == KEYUP) {
    switch (event.e.key) {
      case GLFW_KEY_RIGHT_SHIFT: {
        state.rshift_hold = false;
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
            state.selected_geo_id = geo;
            state.camera.mouse_enable = false;

            state.selected_geo = state_get_geometry(geo);
            panic(state.selected_geo, "Selected Geometry is NULL");

            state.selected_color = (v4) {
              state.selected_geo->color.x,
              state.selected_geo->color.y,
              state.selected_geo->color.z,
              1
            };
          }
        } else {
          state.mouse_clicked = true;
        }
      } break;

      case MOUSE_BUTTON_RIGHT: {
        if (state.current_control == CON_CAMERA) {
          state.camera.mouse_enable = false;
          state.current_control = CON_SPAWN;
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

  editor_movement_event(event);
}

void state_update() {
  // Camera control using mouse
  pcamera_handle_mouse(&state.camera, state.window);

  switch (state.current_control) {
    case CON_CAMERA:
      editor_camera_movement_update();
      break;
    case CON_GEOMETRY:
      editor_geometry_movement_update();
      break;
    case CON_SETTINGS:
      editor_settings_update();
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
  switch (state.current_control) {
    case CON_SPAWN:
      pannel_spawn_geometry();
      break;
    case CON_GEOMETRY:
      pannel_geometry_data();
      pannel_geometry_help();
      pannel_geometry_color();
      break;
    case CON_SETTINGS:
      pannel_settings();
      break;
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
    (v4) { 1, 1, 1, 1 }
  );

  // Render FPS
  {
    char text[100];
    snprintf(text, sizeof(text), "FFS: %d", state.fc.fps);
    font_render(
      &state.imr, &state.font, text,
      (v3) { 30, 30, 0 },
      (v4) { 1, 1, 1, 1 }
    );
  }

  // Render mode
  {
    char text[100];
    snprintf(text, sizeof(text), "Mode: %s", control_type_name(state.current_control));
    font_render(
      &state.imr, &state.font, text,
      (v3) { 30, 70, 0 },
      (v4) { 1, 1, 1, 1 }
    );
  }

  // Render Help Controls
  if (state.help) {
    char* controls_text = 
      "WASD        - Movement\n"
      "L-SHIFT     - Go Down\n"
      "SPACE       - Go Up\n"
      "RIGHT-CLICK - Spawn Geometry Mode\n"
      "[Click Obj] - Edit Geometry Mode\n"
      "ESC         - FPS Camera Mode\n"
      "C           - Settings Mode\n"
    ;
    font_render(
      &state.imr, &state.font, controls_text,
      (v3) { state.window.width - 400, 30, 0 },
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

void state_upscale_pass() {
  i32 loc;

  glViewport(0, 0, state.window.width, state.window.height);
  imr_begin(&state.imr);
  {
    Shader shader = state.shaders[SHADER_UPSCALE];
    imr_switch_shader(
      &state.imr,
      shader
    );

    // Bind the raymarch rendered frame
    texture_bind(state.rendered_frame.color_texture);

    loc = GLCall(glGetUniformLocation(shader, "u_texture"));
    GLCall(glUniform1i(loc, state.rendered_frame.color_texture.id));

    loc = GLCall(glGetUniformLocation(shader, "u_resolution"));
    GLCall(glUniform2f(
      loc,
      state.rendered_frame.width,
      state.rendered_frame.height
    ));

    imr_push_quad(
      &state.imr,
      (v3) { -1, -1, 0 },
      (v2) { 2, 2 },
      m4_identity(),
      (v4) { 1, 1, 1, 1 }
    );
  }
  imr_end(&state.imr);
}

void state_render() {
  state_raymarch_pass();
  state_upscale_pass();

  glViewport(0, 0, state.window.width, state.window.height);
  imr_begin(&state.imr);
  {
    imr_switch_shader_to_default(&state.imr);
    m4 mvp = ocamera_calc_mvp(&state.ui_camera);
    imr_update_mvp(&state.imr, mvp);

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

b8 color_selector(Rect rect, v4* selected_color) {
  i32 palette_cols = 8;
  i32 palette_rows = 8;
  f32 cell_spacing = 1.0f;
  b8 selected = false;

  for (i32 i = 0; i < PALETTE_SIZE; i++) {
    i32 row = i / palette_cols;
    i32 col = i % palette_cols;

    f32 x = rect.x + col * (rect.w + cell_spacing);
    f32 y = rect.y + row * (rect.h + cell_spacing);

    if (state.mouse_clicked) {
      v2 mouse = event_mouse_pos(state.window);
      if (point_in_rect(mouse, (Rect) {x, y, rect.w, rect.h})) {
        *selected_color = PALETTE[i];
        selected = true;
      }
    }

    imr_push_quad(
      &state.imr,
      (v3){x, y, 1},
      (v2){rect.w, rect.h},
      m4_identity(),
      PALETTE[i]
    );
  }

  return selected;
}

void pannel_spawn_geometry() {
  f32 gap = 10.0f;
  v2 pannel_size = SPAWN_GEOMETRY_PANNEL_SIZE;
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

void pannel_geometry_data() {
  Geometry* geo = state.selected_geo;
  panic(geo, "Selected Geometry is NULL");

  f32 gap = 10.0f;
  v2 pannel_size = GEOMETRY_DATA_PANNEL_SIZE;
  v3 pannel_pos = {
    state.window.width / 2.0f - pannel_size.x / 2.0f,
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

  // Label starting pos
  f32 label_gap = 15;
  char* tmp_text = "A";
  v2 label_size = font_calc_size(&state.font, tmp_text);
  f32 x = pannel_pos.x + label_gap;
  f32 y = pannel_pos.y + label_size.y + label_gap;

  // Render id
  {
    char text[100];
    snprintf(
      text, sizeof(text),
      "Id        : %d",
      geo->id
    );
    font_render(
      &state.imr,
      &state.font,
      text,
      (v3) { x, y, 1 },
      (v4) { 1, 1, 1, 1 }
    );
    y += label_size.y + label_gap;
  }

  // Render position
  {
    char text[100];
    snprintf(
      text, sizeof(text),
      "Pos       : (%.2f, %.2f, %.2f)",
      geo->center.x, geo->center.y, geo->center.z
    );
    font_render(
      &state.imr,
      &state.font,
      text,
      (v3) { x, y, 1 },
      (v4) { 1, 1, 1, 1 }
    );
    y += label_size.y + label_gap;
  }

  // Render color
  {
    char text[100];
    snprintf(
      text, sizeof(text),
      "Color     : (%.2f, %.2f, %.2f) ",
      geo->color.x, geo->color.y, geo->color.z
    );
    v2 size = font_calc_size(&state.font, text);
    font_render(
      &state.imr,
      &state.font,
      text,
      (v3) { x, y, 1 },
      (v4) { 1, 1, 1, 1 }
    );
    imr_push_quad(
      &state.imr,
      (v3) { x + size.x, y - label_size.y, 1 },
      (v2) { label_size.y, label_size.y },
      m4_identity(),
      (v4) { geo->color.x, geo->color.y, geo->color.z, 1 }
    );
    y += label_size.y + label_gap;
  }

  // Render based on type
  switch (geo->type) {
    case GEO_SPHERE: {
      char text[100];
      snprintf(
        text, sizeof(text),
        "Radius    : %.2f", geo->sphere.radius
      );
      font_render(
        &state.imr,
        &state.font,
        text,
        (v3) { x, y, 1 },
        (v4) { 1, 1, 1, 1 }
      );
      y += label_size.y + label_gap;
    } break;

    case GEO_BOX: {
      char text[100];
      snprintf(
        text, sizeof(text),
        "Half Size : (%.2f, %.2f, %.2f)",
        geo->box.half_size.x, geo->box.half_size.y, geo->box.half_size.z
      );
      font_render(
        &state.imr,
        &state.font,
        text,
        (v3) { x, y, 1 },
        (v4) { 1, 1, 1, 1 }
      );
      y += label_size.y + label_gap;
    } break;
  }
}

void pannel_geometry_help() {
  f32 gap = 10.0f;
  v2 pannel_size = GEOMETRY_HELP_PANNEL_SIZE;
  v3 pannel_pos = {
    state.window.width / 2.0f -
    GEOMETRY_DATA_PANNEL_SIZE.x / 2.0f -
    pannel_size.x - gap,
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

  // Render help text
  const char* text =
    "A-D              - X movement\n"
    "W-S              - Z movement\n"
    "SPACE-LSHIFT     - Y movement\n"
    "[UP Arrow]       - Y size increment\n"
    "[DOWN Arrow]     - Z size increment\n"
    "[RIGHT Arrow]    - X size increment\n"
    "RSHIFT + [Arrow] - size decrement\n"
    "DELETE           - Delete geometry"
  ;

  v2 label_size = font_calc_size(&state.font, text);
  font_render(
    &state.imr,
    &state.font,
    text,
    (v3) {
      pannel_pos.x + 15.0f,
      pannel_pos.y + label_size.y + 15.0f,
      1.0f
    },
    (v4) { 1, 1, 1, 1 }
  );
}

void pannel_geometry_color() {
  f32 gap = 10.0f;
  v2 pannel_size = GEOMETRY_COLOR_PANNEL_SIZE;
  v3 pannel_pos = {
    state.window.width / 2.0f +
    GEOMETRY_DATA_PANNEL_SIZE.x / 2.0f + gap,
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

  f32 item_gap = 15.0f;
  f32 cell_size = 20.0f;
  v2 pos = { pannel_pos.x + item_gap, pannel_pos.y + item_gap };

  // Change the color of geometry
  if (color_selector((Rect) {
    pos.x, pos.y, cell_size, cell_size
  }, &state.selected_color)) {
    state.selected_geo->color = (v3) {
      state.selected_color.r,
      state.selected_color.g,
      state.selected_color.b
    };
  }

  f32 right_start_x = pos.x + 8 + 20 * 8 + 2 * item_gap;

  // Render RGB Values
  char text[100];
  snprintf(
    text, sizeof(text),
    "R: %.2f\nG: %.2f\nB: %.2f",
    state.selected_color.x, state.selected_color.y, state.selected_color.z
  );
  v2 size = font_calc_size(&state.font, text);
  font_render(
    &state.imr,
    &state.font,
    text,
    (v3) { right_start_x, pos.y + item_gap, 1 },
    (v4) { 1, 1, 1, 1 }
  );

  // Render selected color
  imr_push_quad(
    &state.imr,
    (v3) { right_start_x, pos.y + size.y * 5 + item_gap, 1 },
    (v2) { 65, 65 },
    m4_identity(),
    state.selected_color
  );
}

void pannel_settings() {
  f32 gap = 10.0f;
  f32 item_gap = 15.0f;
  v2 pannel_size = SETTINGS_PANNEL_SIZE;
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
  const char* label = "Settings";
  v2 label_size = font_calc_size(&state.font, label);
  v3 label_pos = {
    pannel_pos.x + pannel_size.x / 2 - label_size.x / 2,
    pannel_pos.y + label_size.y + item_gap,
    1
  };
  font_render(
    &state.imr,
    &state.font,
    label,
    label_pos,
    (v4) { 1, 1, 1, 1 }
  );

  v3 start_pos = {
    pannel_pos.x + item_gap,
    label_pos.y + label_size.y * 3 + item_gap,
    1
  };
  v2 button_size = { 130, 30 };

  // Render Operation Type
  {
    const char* label = "Operation: ";
    v2 label_size = font_calc_size(&state.font, label);
    font_render(
      &state.imr,
      &state.font,
      label,
      start_pos,
      (v4) { 1, 1, 1, 1 }
    );

    if (button(render_op_name(state.render_op), (Rect) {
      start_pos.x + label_size.x + item_gap,
      start_pos.y - label_size.y / 2 - button_size.y / 2,
      button_size.x, button_size.y
    })) {
      state.render_op = (state.render_op == OP_UNION) ? OP_INTERSECTION: OP_UNION;
    }

    start_pos.y += button_size.y + item_gap;
  }

  // Render Smoothness
  {
    char label[100];
    snprintf(label, sizeof(label), "Smoothness: %.2f", state.op_smoothness);
    v2 label_size = font_calc_size(&state.font, label);
    font_render(
      &state.imr,
      &state.font,
      label,
      start_pos,
      (v4) { 1, 1, 1, 1 }
    );
    start_pos.y += button_size.y + item_gap;
  }

  // Divider
  {
    imr_push_quad(
      &state.imr,
      (v3) { pannel_pos.x, start_pos.y, 1 },
      (v2) { pannel_size.x, 2.0f },
      m4_identity(),
      (v4) { 1, 1, 1, 1 }
    );
    start_pos.y += button_size.y + item_gap;
  }

  // Render Help
  {
    const char* label =
      "[UP Arrow]   - Inc Smoothness\n"
      "[DOWN Arrow] - Dec Smoothness"
    ;
    v2 label_size = font_calc_size(&state.font, label);
    font_render(
      &state.imr,
      &state.font,
      label,
      start_pos,
      (v4) { 1, 1, 1, 1 }
    );
  }

}

// :editor impl
void editor_movement_event(Event event) {
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
      case GLFW_KEY_UP:
        state.size[UP] = true;
        break;
      case GLFW_KEY_DOWN:
        state.size[DOWN] = true;
        break;
      case GLFW_KEY_RIGHT:
        state.size[RIGHT] = true;
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
      case GLFW_KEY_UP:
        state.size[UP] = false;
        break;
      case GLFW_KEY_DOWN:
        state.size[DOWN] = false;
        break;
      case GLFW_KEY_RIGHT:
        state.size[RIGHT] = false;
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

void editor_geometry_movement_update() {
  Geometry* geo = state.selected_geo;
  panic(geo, "Selected Geometry is NULL");

  // Movement
  if (state.movement[FRONT]) {
    geo->center.z += SPEED * state.fc.dt;
  }
  if (state.movement[BACK]) {
    geo->center.z -= SPEED * state.fc.dt;
  }
  if (state.movement[LEFT]) {
    geo->center.x += SPEED * state.fc.dt;
  }
  if (state.movement[RIGHT]) {
    geo->center.x -= SPEED * state.fc.dt;
  }
  if (state.movement[UP]) {
    geo->center.y += SPEED * state.fc.dt;
  }
  if (state.movement[DOWN]) {
    geo->center.y -= SPEED * state.fc.dt;
  }

  // Scaling
  STATIC_ASSERT(GEO_TOTAL == 3, "Update here!");
  switch (geo->type) {
    case GEO_SPHERE: {
      if (
        state.size[UP] ||
        state.size[DOWN] ||
        state.size[RIGHT]
      ) {
        if (state.rshift_hold) {
          geo->sphere.radius -= GEOMETRY_SCALE_SPEED;
        } else {
          geo->sphere.radius += GEOMETRY_SCALE_SPEED;
        }
      }
    } break;

    case GEO_BOX: {
      if (state.size[UP]) {
        if (state.rshift_hold) {
          geo->box.half_size.y -= GEOMETRY_SCALE_SPEED;
        } else {
          geo->box.half_size.y += GEOMETRY_SCALE_SPEED;
        }
      }
      if (state.size[DOWN]) {
        if (state.rshift_hold) {
          geo->box.half_size.z -= GEOMETRY_SCALE_SPEED;
        } else {
          geo->box.half_size.z += GEOMETRY_SCALE_SPEED;
        }
      }
      if (state.size[RIGHT]) {
        if (state.rshift_hold) {
          geo->box.half_size.x -= GEOMETRY_SCALE_SPEED;
        } else {
          geo->box.half_size.x += GEOMETRY_SCALE_SPEED;
        }
      }
    } break;
  }
}

void editor_settings_update() {
  if (state.size[UP]) {
    state.op_smoothness += SMOOTHNESS_SPEED;
    if (state.op_smoothness >= 1.0f)
      state.op_smoothness = 1.0f;
  }
  if (state.size[DOWN]) {
    state.op_smoothness -= SMOOTHNESS_SPEED;
    if (state.op_smoothness <= 0.0f)
      state.op_smoothness = 0.0f;
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

    // Reset the clicked events at the end of frame
    state.mouse_clicked = false;
  }

  state_clean();
  return 0;
}
