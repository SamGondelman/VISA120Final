#version 400 core

layout(location = 0) in vec3 position; // Position of the vertex
layout(location = 1) in vec3 normal;   // Normal of the vertex

uniform sampler2D pos;
uniform sampler2D vel;
uniform int numParticles;
uniform int particleID;

out vec4 viewPos;
out vec3 nor;

// Transformation matrices
uniform mat4 P;
uniform mat4 V;

void main() {
    vec4 posTime = texelFetch(pos, ivec2(particleID, 0), 0);
    vec4 velAge = texelFetch(vel, ivec2(particleID, 0), 0);

    // Calculate diameter based on age and lifetime
    float diameter = 0.02;
    diameter *= min(min(1.0, max(0, velAge.w) / (0.1 * posTime.w)),
                    min(1.0, abs(posTime.w - max(0, velAge.w)) / (0.1 * posTime.w)));

    // The offset to the points of the triangle
    vec4 triPos = diameter * vec4(position, 1);

    // Anchor point in world space
    vec4 anchorPoint = vec4(posTime.xyz, 1.0);

    // Center the particle around anchorPoint
    viewPos = V * (anchorPoint + triPos - diameter * vec4(0.5, 0.5, 0.0, 0.0));
    nor = normalize(mat3(transpose(inverse(V))) * normal);
    gl_Position = P * viewPos;
}
