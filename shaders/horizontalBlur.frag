#version 400 core

in vec2 texCoord;

uniform sampler2D tex;

out vec4 fragColor;

void main(){
    vec2 texelSize = 1.0 / textureSize(tex, 0).xy;
    const int supportWidth = 5;

    float weights = 0.0;
    for (int i = -supportWidth; i <= supportWidth; i++) {
        float weight = (supportWidth + 1) - abs(i);
        fragColor += weight * texture(tex, texCoord + vec2(i * texelSize.x, 0));
        weights += weight;
    }
    fragColor /= weights;
}
