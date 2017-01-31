#version 400 core

in vec4 viewPos;
in vec3 nor;
in float ageFrac;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 fragNor;
layout(location = 2) out vec4 fragAmb;
layout(location = 3) out vec4 fragDiff;
layout(location = 4) out vec4 fragSpec;

vec3 calcColor() {
    const int NUM_COLORS = 3;
    const vec3 colors[NUM_COLORS] = vec3[NUM_COLORS](
        vec3(1, 0.71, 0.129),
        vec3(0.92, 0.22, 0.14),
        vec3(0.333, 0.2, 0.267)
    );
    float p = (NUM_COLORS - 1) * ageFrac;
    return 0.5*mix(colors[int(p)], colors[min(NUM_COLORS - 1, int(p+1))], fract(p));
}

void main() {
    fragPos = viewPos;
    fragNor = vec4(nor, 1);
    fragAmb = vec4(calcColor(), 1);
    fragDiff = vec4(12*(1-ageFrac)*calcColor(), 1);
    fragSpec = vec4(4*(1-ageFrac)*calcColor(), 5.0);
}
