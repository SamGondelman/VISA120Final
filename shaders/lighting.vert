#version 400 core

layout(location = 0) in vec3 position; // Position of the vertex

// Transformation matrices
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform float worldSpace;

void main() {
    vec4 pos = vec4(position, 1);
    gl_Position = worldSpace * (P * V * M * pos) + (1 - worldSpace) * pos;
}
