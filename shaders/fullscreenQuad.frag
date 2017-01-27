#version 330 core

in vec2 texCoord;

uniform sampler2D pos;
uniform sampler2D nor;
uniform sampler2D amb;
uniform sampler2D diff;
uniform sampler2D spec;

out vec4 fragColor;

void main() {
    //fragColor = vec4(texCoord, 0, 1);
    fragColor = texture(pos, texCoord);
    fragColor = texture(nor, texCoord);
    fragColor = texture(amb, texCoord);
    fragColor = texture(diff, texCoord);
    fragColor = texture(spec, texCoord);
    fragColor = texture(pos, texCoord);
}
