#version 400 core

in vec4 pos;
in vec2 texc;

uniform sampler2D normalMap;
uniform sampler2D color;
uniform vec2 screenSize;

out vec4 fragColor;

void main() {
    vec2 uv = gl_FragCoord.xy / screenSize;
    vec2 offset = 2 * texture(normalMap, texc).zy - 1;
    const float OFFSET_SCALE = 0.1;
    vec4 SHIELD_COLOR = vec4(vec3(0.1), 1);
    fragColor = mix(texture(color, uv + OFFSET_SCALE * offset), SHIELD_COLOR, 0.1);
}
