#version 400 core

in vec4 pos;
in vec3 nor;

// Material data
uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float shininess;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 fragNor;
layout(location = 2) out vec4 fragAmb;
layout(location = 3) out vec4 fragDiff;
layout(location = 4) out vec4 fragSpec;

void main() {
    fragPos = pos;
    fragNor = vec4(nor, 1);
    fragAmb = vec4(ambient_color, 1);
    fragDiff = vec4(diffuse_color, 1);
    fragSpec = vec4(specular_color, shininess);
}
