#version 400 core

uniform float active;
uniform float firstPass;
uniform sampler2D prevPos;
uniform sampler2D prevVel;
uniform int numParticles;

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
    const float MAX_LIFETIME = 5.0;
    const float MIN_LIFETIME = 0.5;
    return MIN_LIFETIME + (MAX_LIFETIME - MIN_LIFETIME) * hash(index * 2349.2693);
}

vec3 calculateInitialVelocity(int index) {
    float theta = 2.0 * PI * hash(index * 872.0238);
    float phi = PI * hash(index * 1912.124);
    const float MAX_VEL = 0.3;
    float velMag = MAX_VEL * hash(index * 98723.345);
    float sinPhi = sin(phi);
    return velMag * vec3(sinPhi*cos(theta), sinPhi*sin(theta), cos(phi));
}

vec4 initPosition(int index) {
    const vec3 spawn = vec3(1);
    return vec4(spawn, calculateLifetime(index));
}

vec4 initVelocity(int index) {
    return vec4(calculateInitialVelocity(index), -calculateLifetime(index));
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
    const float G = -0.1;
    return vel + vec4(0, G * dt, 0, dt);
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
