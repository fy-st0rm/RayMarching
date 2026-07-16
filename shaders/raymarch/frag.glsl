#version 330 core

layout (location = 0) out vec4 color;

in vec2 screenCoord;

struct Sphere {
  vec3 center;
  float radius;
  vec3 color;
};
Sphere spheres[32];
int spheres_count = 0;

struct SceneResult {
  float distance;
  vec3 color;
};

float sphereSDF(vec3 p, vec3 center, float radius) {
  return length(p - center) - radius;
}

SceneResult sphere(vec3 p, Sphere sphere) {
  SceneResult result;
  result.distance = sphereSDF(p, sphere.center, sphere.radius);
  result.color = sphere.color;
  return result;
}

SceneResult unionOp(SceneResult a, SceneResult b) {
  if (a.distance < b.distance) return a;
  return b;
}

SceneResult smoothUnion(SceneResult a, SceneResult b, float k) {
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

  for (int i = 0; i < spheres_count; i++) {
    result = smoothUnion(result, sphere(p, spheres[i]), 0.4);
  }

  return result;
}

vec3 calculateNormal(vec3 p)
{
  const float e = 0.001;

  return normalize(vec3(
    scene(p + vec3(e,0,0)).distance - scene(p - vec3(e,0,0)).distance,
    scene(p + vec3(0,e,0)).distance - scene(p - vec3(0,e,0)).distance,
    scene(p + vec3(0,0,e)).distance - scene(p - vec3(0,0,e)).distance
  ));
}

SceneResult rayMarch(vec3 rayOrigin, vec3 rayDirection) {
  SceneResult final_result;
  final_result.distance = -1;
  final_result.color = vec3(0);

  float travel = 0.0;

  for (int i = 0; i < 100; i++) {
    vec3 samplePoint = rayOrigin + rayDirection * travel;

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
  spheres[spheres_count++] = Sphere(
    vec3(0, 0.0, 0.0),
    1.0,
    vec3(1.0, 0.0, 0.0)
  );

  spheres[spheres_count++] = Sphere(
    vec3(1.5, 0.0, 0.0),
    1.0,
    vec3(1.0, 1.0, 0.0)
  );

  vec3 rayOrigin = vec3(0.0, 0.0, -3.0);
  vec3 rayDirection = normalize(vec3(screenCoord, 1.0));

  SceneResult result = rayMarch(rayOrigin, rayDirection);
  if (result.distance < 0) {
    color = vec4(result.color, 1);
  } else {
    vec3 hitPoint = rayOrigin + rayDirection * result.distance;
    vec3 normal = calculateNormal(hitPoint);
    vec3 lightDir = normalize(vec3(1,1,-1));
    float diffuse = max(dot(normal, lightDir),0.0);

    color = vec4(result.color * vec3(diffuse),1);
  }
}
