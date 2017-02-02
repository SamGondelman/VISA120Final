#version 400 core

in vec4 pos;
in vec3 nor;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 fragNor;
layout(location = 2) out vec4 fragAmb;
layout(location = 3) out vec4 fragDiff;
layout(location = 4) out vec4 fragSpec;

void main() {
    fragPos = pos;
    fragNor = vec4(nor, 1);
    fragAmb = vec4(0.25f*vec3(0.137, 0.094, 0.118), 1);
    fragDiff = vec4(0.25f*vec3(0.443, 0.263, 0.2), 1);
    fragSpec = vec4(0.25f*vec3(0.773, 0.561, 0.419), 1.0);
}
