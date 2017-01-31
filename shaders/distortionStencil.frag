#version 400 core

in vec2 texc;

uniform sampler2D normalMap;

out vec4 fragColor;

void main() {
    vec2 offset = 2 * texture(normalMap, texc).zy - 1;
    const float OFFSET_SCALE = 0.05;
    fragColor = vec4(OFFSET_SCALE * offset, 1, 1);
}
