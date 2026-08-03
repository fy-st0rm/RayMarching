#version 330 core

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 ids;
in vec2 screen_coord;

uniform vec3 u_camera_pos;
uniform vec3 u_camera_forward;
uniform vec3 u_camera_right;
uniform vec3 u_camera_up;

// Config
#define MAX_ITERATION 200
#define MAX_GEOMETRY 32

struct SceneResult {
  int id;
  float distance;
  vec3 color;
};

// Geometries
#define GEO_NONE   0
#define GEO_SPHERE 1
#define GEO_BOX    2
struct Geometry {
  int type;
  int id;
  vec3 color;
  vec3 center;
  vec4 data;
};

uniform Geometry geometries[MAX_GEOMETRY];
uniform int geometries_count;


// :sphere
SceneResult sphere(vec3 p, int id, vec3 center, float radius, vec3 color) {
  SceneResult result;
  result.id = id;
  result.distance = length(p - center) - radius;
  result.color = color;
  return result;
}

// :box
SceneResult box(vec3 p, int id, vec3 center, vec3 half_size, vec3 color) {
  vec3 q = abs(p - center) - half_size;

  SceneResult result;
  result.id = id;
  result.distance = length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
  result.color = color;
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
  if (a.distance < b.distance)
      result.id = a.id;
  else
      result.id = b.id;
  result.distance = mix(b.distance, a.distance, h) - k * h * (1.0 - h);
  result.color = mix(b.color, a.color, h);

  return result;
}

SceneResult scene(vec3 p) {
  SceneResult result;
  result.id = GEO_NONE;
  result.distance = 1e20;
  result.color = vec3(0);

  for (int i = 0; i < geometries_count; i++) {
    SceneResult temp;

    switch (geometries[i].type) {
    case GEO_SPHERE:
        temp = sphere(
            p,
            geometries[i].id,
            geometries[i].center,
            geometries[i].data.x,
            geometries[i].color
        );
        break;
    
    case GEO_BOX:
        temp = box(
            p,
            geometries[i].id,
            geometries[i].center,
            geometries[i].data.xyz,
            geometries[i].color
        );
        break;
    }

    result = union_op(result, temp);
  }

  return result;
}

// Ray march algorithm
SceneResult ray_march(vec3 ray_origin, vec3 ray_direction) {
  SceneResult final_result;
  final_result.id = GEO_NONE;
  final_result.distance = -1;
  final_result.color = vec3(0);

  float travel = 0.0;

  for (int i = 0; i < MAX_ITERATION; i++) {
    vec3 samplePoint = ray_origin + ray_direction * travel;

    SceneResult result = scene(samplePoint);

    if (result.distance < 0.01) {
      final_result.id = result.id;
      final_result.distance = travel;
      final_result.color = result.color;
      break;
    }

    travel += result.distance;
  }

  return final_result;
}

// Light stuff
vec3 calculate_normal(vec3 p)
{
  const float e = 0.001;

  return normalize(vec3(
    scene(p + vec3(e,0,0)).distance - scene(p - vec3(e,0,0)).distance,
    scene(p + vec3(0,e,0)).distance - scene(p - vec3(0,e,0)).distance,
    scene(p + vec3(0,0,e)).distance - scene(p - vec3(0,0,e)).distance
  ));
}

float soft_shadow(vec3 ro, vec3 rd) {
  float res = 1.0;
  float t = 0.02;

  for(int i = 0; i < 32; i++) {
    float h = scene(ro + rd * t).distance;
    
    if(h < 0.001) return 0.0;

    res = min(res, 8.0 * h / t);
    t += clamp(h, 0.01, 0.2);

    if(t > 20.0) break;
  }
  return clamp(res, 0.0, 1.0);
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
    color = vec4(0, 0, 0, 1);
    ids = vec4(0, 0, 0, 0);
  } else {
    vec3 hitPoint = ray_origin + ray_direction * result.distance;
    vec3 normal = calculate_normal(hitPoint);
    vec3 lightPos = vec3(5.0, 6.0, -4.0);
    vec3 lightDir = normalize(lightPos - hitPoint);
    vec3 viewDir = normalize(ray_origin - hitPoint);

    // Ambient
    float ambient = 0.18;

    // Diffuse
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Blinn-Phong Specular
    vec3 halfVec = normalize(lightDir + viewDir);
    float specular = pow(max(dot(normal, halfVec), 0.0), 164.0);

    // Soft shadow
    float shadow = soft_shadow(hitPoint + normal * 0.01, lightDir);
    diffuse *= shadow;
    specular *= shadow;

    // Final lighting
    float lighting = ambient + diffuse * 0.9 + specular * 0.35;
    vec3 finalColor = result.color * lighting;

    // Gamma correction
    finalColor = pow(finalColor, vec3(1.0 / 2.2));
    color = vec4(finalColor, 1);
    ids = vec4(float(result.id) / 255.0, 0.0, 0.0, 1.0);
  }
}
