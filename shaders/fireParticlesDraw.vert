#version 400 core

uniform sampler2D pos;
uniform sampler2D vel;
uniform int numParticles;

out vec4 viewPos;
out vec3 nor;
out float ageFrac;

// Transformation matrices
uniform mat4 P;
uniform mat4 V;

vec3 rotate(vec3 v, vec3 k, float theta) {
    return v * cos(theta) + cross(k, v) * sin(theta) + k * dot(k, v) * (1 - cos(theta));
}

void main() {
    vec4 posTime = texelFetch(pos, ivec2(gl_InstanceID, 0), 0);
    vec4 velAge = texelFetch(vel, ivec2(gl_InstanceID, 0), 0);

    // Calculate diameter based on age and lifetime
    float diameter = 0.025;
    diameter *= min(min(1.0, max(0, velAge.w) / (0.1 * posTime.w)),
                    min(1.0, abs(posTime.w - max(0, velAge.w)) / (0.1 * posTime.w)));

    ageFrac = min(1, max(0, velAge.w) / posTime.w);

    vec3 unitV = normalize(velAge.xyz);
    float isUP = ceil(max(0, unitV.y - 0.99));
    const float TWO_PI_THREE = 2.094395;
    vec3 p2 = normalize(cross(unitV, (1-isUP) * vec3(0, 1, 0) + isUP * vec3(1, 0, 0)));
    vec3 p3 = rotate(p2, unitV, TWO_PI_THREE);
    vec3 p4 = rotate(p2, unitV, -TWO_PI_THREE);
    vec4 OFFSETS[10] = vec4[10](
        vec4(2 * unitV, 0),
        vec4(p2, 0),
        vec4(p3, 0),
        vec4(p4, 0),
        vec4(p2, 0),
        vec4(-2 * unitV, 0),
        vec4(p2, 0),
        vec4(p4, 0),
        vec4(p3, 0),
        vec4(p2, 0)
    );

    // The offset to the points of the triangle
    vec4 triPos = diameter * OFFSETS[gl_VertexID];

    // Anchor point in world space
    vec4 anchorPoint = vec4(posTime.xyz, 1.0);

    // Center the particle around anchorPoint
    viewPos = V * (anchorPoint + triPos - diameter * vec4(0.5, 0.5, 0.0, 0.0));
    nor = normalize(mat3(transpose(inverse(V))) * normalize(OFFSETS[gl_VertexID].xyz));
    gl_Position = P * viewPos;
}
