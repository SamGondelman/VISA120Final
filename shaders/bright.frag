#version 400 core

in vec2 texCoord;

uniform sampler2D color;

out vec4 brightColor;

void main() {
    vec3 col = texture(color, texCoord).xyz;
    float threshold = 1.0f;
    float brightenRatio = 1.0f / (max(max(col.r, col.g), col.b) + 0.0001);
    brightenRatio *= step(threshold, dot(col, vec3(0.2126, 0.7152, 0.0722)));
    col *= brightenRatio;
    brightColor = vec4(col, 1);
}
