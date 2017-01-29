#version 400 core

layout(location = 0) in vec3 position; // Position of the vertex
layout(location = 1) in vec3 normal;   // Normal of the vertex
layout(location = 5) in vec2 texCoord; // UV texture coordinates

out vec4 pos;
out vec3 nor;
out vec2 texc;
out vec3 edge;

// Transformation matrices
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main() {
    pos = V * M * vec4(position, 1.0);
    nor = normalize(mat3(transpose(inverse(V * M))) * normal);
    texc = texCoord;
    edge = vec3(0);
    edge[gl_VertexID % 3] = 1;
    gl_Position = P * pos;
}
