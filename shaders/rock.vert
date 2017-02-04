#version 400 core

layout(location = 0) in vec3 position; // Position of the vertex
layout(location = 1) in vec3 normal;   // Normal of the vertex

uniform float time;

out vec4 pos;
out vec3 nor;
out vec3 edge;

// Transformation matrices
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

float hash(float n) { return fract(sin(n)*753.5453123); }

void main() {
    const vec3 origin = vec3(0, -0.5, 0);
    const vec3 axis = vec3(0, 1, 0);
    vec3 n = 0.025 * (2 * vec3(hash(position.x * position.z),
                             hash(position.z * position.z),
                             hash(position.z * position.x * position.z)) - 1);
    vec3 mPos = position + n;
    vec3 a = (mPos - origin);
    float axisLength = dot(a, axis);
    vec3 a1 = axisLength * axis;
    vec3 radial = a - a1;
    float RAD_MAG = min(max(0, 1.5 * sqrt(time - axisLength * axisLength) - 0.5 * hash(int(gl_VertexID / 3))), 1);
    mPos = origin + (a1 + RAD_MAG * radial);

    pos = V * M * vec4(mPos, 1);
    nor = normalize(mat3(transpose(inverse(V * M))) * normal);
    edge = vec3(0);
    edge[gl_VertexID % 3] = 1;
    gl_Position = P * pos;
}
