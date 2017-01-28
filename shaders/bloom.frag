#version 400 core

in vec2 texCoord;

uniform sampler2D color;
uniform sampler2D bloom;
uniform float exposure;

out vec3 fragColor;

vec3 tonemap(vec3 color) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

void main(){
    vec3 hdrColor = texture(color, texCoord).xyz;
    vec3 bloomColor = texture(bloom, texCoord).xyz;
    hdrColor += bloomColor;

    hdrColor = tonemap(hdrColor * exposure) / tonemap(vec3(1));

    fragColor = hdrColor;
}
