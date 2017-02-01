#version 400 core

layout(location = 0) in vec3 position; // Position of the vertex
layout(location = 1) in vec3 normal;   // Normal of the vertex

uniform float time;
uniform vec3 origin;
const vec3 axis = vec3(1, 0, 0);

out vec4 pos;
out vec3 nor;

// Transformation matrices
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

float hash(float n) { return fract(sin(n)*753.5453123); }

void main() {
    vec3 n = 0.025 * (2 * vec3(hash(position.x * position.z),
                             hash(position.z * position.z),
                             hash(position.z * position.x * position.z)) - 1);
    vec3 wPos = vec3(M * vec4(position + n, 1.0));
    vec3 a = (wPos - origin);
    float axisLength = dot(a, axis);
    vec3 a1 = axisLength * axis;
    vec3 radial = a - a1;
    float RAD_MAG = min(max(0, 1.5 * sqrt(time - axisLength * axisLength) - 0.5 * hash(int(gl_VertexID / 3))), 1);
    wPos = origin + (a1 + RAD_MAG * radial);

    pos = V * vec4(wPos, 1);
    nor = normalize(mat3(transpose(inverse(V * M))) * normal);
    gl_Position = P * pos;
}
