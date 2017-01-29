#version 400 core

in vec4 pos;
in vec3 nor;
in vec2 texc;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 fragNor;
layout(location = 2) out vec4 fragAmb;

uniform mat4 V;
uniform vec3 origin;
uniform float time;

float lineIntensity(float p) {
    const float LINE_RADIUS = 0.05;
    return 1 - min(abs(p - float(round(p))), LINE_RADIUS)/LINE_RADIUS;
}

vec3 lightWorld() {
    vec3 wPos = vec3(inverse(V) * pos);

    float line = max(lineIntensity(wPos.x), max(lineIntensity(wPos.y), lineIntensity(wPos.z)));

    vec3 dist = wPos - origin;

    const float RING_SPEED = 5.0f;
    const float RING_SIZE = 3.0f;
    float ringTime = RING_SPEED * time;
    line *= 1 - min(abs(length(dist) - ringTime + RING_SIZE), RING_SIZE)/RING_SIZE;

    const float RING_DURATION = 5.0f * RING_SPEED;
    float LINE_INTENSITY = 0.4f * (1 - ringTime / RING_DURATION);
    return LINE_INTENSITY * vec3(line);
}

void main() {
    fragPos = pos;
    fragNor = vec4(nor, 1);
    fragAmb = vec4(lightWorld(), 1);
}
