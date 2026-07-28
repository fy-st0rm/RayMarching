#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

out vec2 screen_coord;

uniform vec2 u_resolution;

void main() {
  screen_coord = position.xy;
  screen_coord.x *= u_resolution.x / u_resolution.y;
  gl_Position = vec4(position, 1.0f);
}
