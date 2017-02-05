#version 400 core

uniform float active;
uniform float firstPass;
uniform sampler2D prevPos;
uniform sampler2D prevVel;
uniform int numParticles;

const vec3 spawn = vec3(-1, 0.75, 0);
const vec3 dir = normalize(vec3(1, 0, 1));

in vec2 texCoord;

layout(location = 0) out vec4 pos;
layout(location = 1) out vec4 vel;

const float PI = 3.14159;
const float dt = 0.0167; // 1 sec/60 fps

/*
    A particle has the following properties:
        - pos.xyz = clip space position
        - pos.w = lifetime in seconds (doesn't change)
        - vel.xyz = clip space velocity
        - vel.w = current age in seconds
*/

// A helpful procedural "random" number generator
float hash(float n) { return fract(sin(n)*753.5453123); }

// Helper functions to procedurally generate lifetimes and initial velocities
// based on particle index
float calculateLifetime(int index) {
    const float MAX_LIFETIME = 1.5;
    const float MIN_LIFETIME = 0.5;
    return MIN_LIFETIME + (MAX_LIFETIME - MIN_LIFETIME) * hash(index * 2349.2693);
}

vec4 initPosition(int index) {
    float theta = 2.0 * PI * hash(index * 872.0238);
    float phi = PI * hash(index * 1912.124);
    const float MAX_OFFSET = 0.1;
    float offsetMag = MAX_OFFSET * hash(index * 98723.345);
    float sinPhi = sin(phi);
    return vec4(spawn + offsetMag * vec3(sinPhi*cos(theta), sinPhi*sin(theta), cos(phi)),
                calculateLifetime(index));
}

vec4 initVelocity(int index) {
    const float VEL_MAG = 3.0;
    return vec4(VEL_MAG * dir, -calculateLifetime(index));
}

vec4 updatePosition(int index, vec4 pos, vec4 vel) {
    if (vel.w < 0) {
        vel = vec4(0);
    }
    return vec4(pos.xyz + vel.xyz * dt, pos.w);
}

vec4 updateVelocity(int index, vec4 pos, vec4 vel) {
    if (vel.w < 0) {
        return vel + vec4(0, 0, 0, dt);
    }
    vec3 a = (pos.xyz - spawn);
    vec3 a1 = dot(a, dir) * dir;
    float RAD_MAG = 20.0 * hash(index * 2935.0233);
    vec3 radial = a - a1;

    float tanHash = hash(index * 5234.723);
    float TAN_MAG = 0.5 * tanHash * tanHash;
    vec3 tangential = normalize(cross(radial, dir));

    return vel + dt*vec4(-RAD_MAG*radial + TAN_MAG*tangential, 0) + vec4(0, 0, 0, dt);
}

void main() {
    int index = int(texCoord.x * numParticles);
    if (firstPass > 0.5) {
        pos = initPosition(index);
        vel = initVelocity(index);
    } else {
        pos = texture(prevPos, texCoord);
        vel = texture(prevVel, texCoord);
        if (active > 0.5 || (active < 0.5 && vel.w > 0 && vel.w < pos.w)) {
            pos = updatePosition(index, pos, vel);
            vel = updateVelocity(index, pos, vel);
        }

        if (pos.w < vel.w) {
            pos = initPosition(index);
            vel = initVelocity(index);
        }
    }
}
