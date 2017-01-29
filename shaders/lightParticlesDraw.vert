#version 400 core

uniform sampler2D pos;
uniform sampler2D vel;
uniform int numParticles;

out vec4 viewPos;
out vec2 uv;

// Transformation matrices
uniform mat4 P;
uniform mat4 V;

// Offsets and UVs for the triangle around the particle
const int NUM_VERTICES_PER_PARTICLE = 3;
const vec4 TRI_VERTS[NUM_VERTICES_PER_PARTICLE] = vec4[NUM_VERTICES_PER_PARTICLE](
    vec4(0, 2.0, 0.0, 0.0),
    vec4(0.0, 0.0, 0.0, 0.0),
    vec4(2.0, 0.0, 0.0, 0.0)
);

void main() {
    int particleID = gl_VertexID / NUM_VERTICES_PER_PARTICLE;
    int triID = gl_VertexID - particleID * NUM_VERTICES_PER_PARTICLE;

    // Pass the tex coords to the fragment shader
    uv = TRI_VERTS[triID].xy;

    vec4 posTime = texelFetch(pos, ivec2(particleID, 0), 0);
    vec4 velAge = texelFetch(vel, ivec2(particleID, 0), 0);

    // Calculate diameter based on age and lifetime
    float diameter = 0.02;
    diameter *= min(min(1.0, velAge.w / (0.1 * posTime.w)),
                    min(1.0, abs(posTime.w - velAge.w) / (0.1 * posTime.w)));

    // The offset to the points of the triangle
    vec4 triPos = diameter * TRI_VERTS[triID];

    // Anchor point in world space
    vec4 anchorPoint = vec4(posTime.xyz, 1.0);

    // Center the particle around anchorPoint
    viewPos = V * (anchorPoint + triPos - diameter * vec4(0.5, 0.5, 0.0, 0.0));
    gl_Position = P * viewPos;
}
