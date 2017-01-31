#version 400 core

in vec2 texCoord;

uniform sampler2D offsetMap;
uniform sampler2D color;

out vec4 fragColor;

void main() {
    vec3 offset = texture(offsetMap, texCoord).xyz;
    float useOffset = texture(offsetMap, texCoord + offset.xy).z;
    vec3 colorClear = texture(color, texCoord).xyz;
    vec3 colorOffset = texture(color, texCoord + offset.xy).xyz;
    vec3 DIST_COLOR = offset.z * vec3(0, 0.05, 0.05);
    fragColor = vec4(mix(colorClear, colorOffset, useOffset) + DIST_COLOR, 1);
}
