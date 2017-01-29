#version 400 core

in vec4 viewPos;
in vec2 uv;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 fragNor;
layout(location = 2) out vec4 fragAmb;
layout(location = 3) out vec4 fragDiff;
layout(location = 4) out vec4 fragSpec;

void main() {
    fragPos = viewPos;
    fragNor = vec4(vec3(1), 1);
    fragAmb = vec4(vec3(1), 1);
    fragDiff = vec4(vec3(1), 1);
    fragSpec = vec4(vec3(1), 1);
}
