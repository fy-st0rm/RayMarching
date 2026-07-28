#version 330 core

layout (location = 0) out vec4 color;
in vec2 screen_coord;

uniform vec3 u_camera_pos;
uniform vec3 u_camera_forward;
uniform vec3 u_camera_right;
uniform vec3 u_camera_up;

struct SceneResult {
  float distance;
  vec3 color;
};

// Geometries
#define MAX_GEOMETRY 32

// :sphere
struct Sphere {
  vec3 center;
  float radius;
  vec3 color;
};
uniform Sphere spheres[MAX_GEOMETRY];
uniform int spheres_count;

float sphere_sdf(vec3 p, vec3 center, float radius) {
  return length(p - center) - radius;
}

SceneResult sphere(vec3 p, Sphere sphere) {
  SceneResult result;
  result.distance = sphere_sdf(p, sphere.center, sphere.radius);
  result.color = sphere.color;
  return result;
}

// :box
struct Box {
  vec3 center;
  vec3 half_size;
  vec3 color;
};
uniform Box boxes[MAX_GEOMETRY];
uniform int boxes_count;

float box_sdf(vec3 p, vec3 center, vec3 half_size) {
  vec3 q = abs(p - center) - half_size;
  return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

SceneResult box(vec3 p, Box box) {
  SceneResult result;
  result.distance = box_sdf(p, box.center, box.half_size);
  result.color = box.color;
  return result;
}

// Ray marching operations
SceneResult union_op(SceneResult a, SceneResult b) {
  if (a.distance < b.distance) return a;
  return b;
}

SceneResult smooth_union(SceneResult a, SceneResult b, float k) {
  float h = clamp(
    0.5 + 0.5 * (b.distance - a.distance) / k,
    0.0,
    1.0
  );

  SceneResult result;
  result.distance = mix(b.distance, a.distance, h) - k * h * (1.0 - h);
  result.color = mix(b.color, a.color, h);

  return result;
}

SceneResult scene(vec3 p) {
  SceneResult result;
  result.distance = 1e20;
  result.color = vec3(0);

  // Spheres
  for (int i = 0; i < spheres_count; i++) {
    result = smooth_union(result, sphere(p, spheres[i]), 0.4);
  }

  // Boxes
  for (int i = 0; i < boxes_count; i++) {
    result = smooth_union(result, box(p, boxes[i]), 0.4);
  }

  return result;
}

vec3 calculate_normal(vec3 p)
{
  const float e = 0.001;

  return normalize(vec3(
    scene(p + vec3(e,0,0)).distance - scene(p - vec3(e,0,0)).distance,
    scene(p + vec3(0,e,0)).distance - scene(p - vec3(0,e,0)).distance,
    scene(p + vec3(0,0,e)).distance - scene(p - vec3(0,0,e)).distance
  ));
}

// Ray march algorithm
SceneResult ray_march(vec3 ray_origin, vec3 ray_direction) {
  SceneResult final_result;
  final_result.distance = -1;
  final_result.color = vec3(0);

  float travel = 0.0;

  for (int i = 0; i < 100; i++) {
    vec3 samplePoint = ray_origin + ray_direction * travel;

    SceneResult result = scene(samplePoint);

    if (result.distance < 0.001) {
      final_result.distance = travel;
      final_result.color = result.color;
      break;
    }

    travel += result.distance;
  }

  return final_result;
}

void main() {
  vec3 ray_origin = u_camera_pos;
  vec3 ray_direction = normalize(
    u_camera_forward +
    screen_coord.x * u_camera_right +
    screen_coord.y * u_camera_up
  );

  SceneResult result = ray_march(ray_origin, ray_direction);
  if (result.distance < 0) {
    color = vec4(result.color, 1);
  } else {
    vec3 hitPoint = ray_origin + ray_direction * result.distance;
    vec3 normal = calculate_normal(hitPoint);
    vec3 lightDir = normalize(vec3(1,1,-1));
    float diffuse = max(dot(normal, lightDir),0.0);

    color = vec4(result.color * vec3(diffuse),1);
  }
}
