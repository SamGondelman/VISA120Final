#version 400 core

in vec4 pos;
in vec3 nor;
in vec3 edge;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 fragNor;
layout(location = 2) out vec4 fragAmb;
layout(location = 3) out vec4 fragDiff;
layout(location = 4) out vec4 fragSpec;

// Material data
uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float shininess;

float lineIntensity(float p) {
    const float LINE_RADIUS = 0.05;
    return 1 - min(abs(p - float(round(p))), LINE_RADIUS)/LINE_RADIUS;
}

vec3 waterWorld() {
    float line = max(lineIntensity(edge.x), max(lineIntensity(edge.y), lineIntensity(edge.z)));
    float LINE_INTENSITY = 0.4f;
    return LINE_INTENSITY * vec3(line*line);
}

void main() {
    fragPos = pos;
    fragNor = vec4(nor, 1);
    fragAmb = vec4(waterWorld() * ambient_color, 1);
    fragDiff = vec4(waterWorld()/* * diffuse_color*/, 1);
    fragSpec = vec4(specular_color, shininess);
}
