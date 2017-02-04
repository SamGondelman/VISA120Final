#version 400 core

in vec4 pos;
in vec3 nor;
in vec3 edge;

uniform float time;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 fragNor;
layout(location = 2) out vec4 fragAmb;
layout(location = 3) out vec4 fragDiff;
layout(location = 4) out vec4 fragSpec;

float lineIntensity(float p) {
    const float LINE_RADIUS = 0.05;
    return 1 - min(abs(p - float(round(p))), LINE_RADIUS)/LINE_RADIUS;
}

vec3 rockLines() {
    float line = max(lineIntensity(edge.x), max(lineIntensity(edge.y), lineIntensity(edge.z)));
    const float TIME_RADIUS = 0.15;
    float LINE_INTENSITY = 0.4f * (1 - min(abs(time - (1.75 - TIME_RADIUS)), TIME_RADIUS)/TIME_RADIUS);
    return LINE_INTENSITY * vec3(line*line);
}

void main() {
    fragPos = pos;
    fragNor = vec4(nor, 1);
    fragAmb = vec4(0.25f*vec3(0.137, 0.094, 0.118) + rockLines(), 1);
    fragDiff = vec4(0.25f*vec3(0.443, 0.263, 0.2), 1);
    fragSpec = vec4(0.25f*vec3(0.773, 0.561, 0.419), 1.0);
}
