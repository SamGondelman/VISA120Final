#version 400 core

// Light properties
struct Light {
    int type;       // 0: point, 1: directional, 3: spot
    vec3 posDir;    // pos for point lights, dir for directional

    // Point lights
    vec3 func;
    float radius;

    vec3 col;
};

uniform Light light;

uniform mat4 V;

// Deferred buffer
uniform sampler2D pos;
uniform sampler2D nor;
uniform sampler2D diff;
uniform sampler2D spec;

uniform float useDiffuse;
uniform float useSpecular;
uniform vec2 screenSize;

out vec3 fragColor;

void main() {
    vec2 texCoord = gl_FragCoord.st/screenSize;

    // Camera space position and normal
    vec3 position = texture(pos, texCoord).xyz;
    vec3 N = texture(nor, texCoord).xyz;
    vec3 E = normalize(-position);

    vec3 diffuse = texture(diff, texCoord).xyz;
    vec4 specular = texture(spec, texCoord);

    vec3 L;
    float att;

    if(light.type == 0){
        L = vec3((V * vec4(light.posDir, 1)) - position);
        float dist = length(L);
        L = normalize(L);
        att = 1.0 / (light.func.x + light.func.y * dist + light.func.z * dist * dist);
    } else if(light.type == 1){
        L = -vec3(transpose(inverse(V)) * vec4(light.posDir, 0));
        att = 1.0;
    }

    // Diffuse
    fragColor += useDiffuse * att * light.col * diffuse * clamp(dot(N, L), 0.0, 1.0);

    // Specular
    vec3 R = normalize(reflect(-L, N));
    fragColor += useSpecular * att * light.col * specular.xyz * pow(clamp(dot(E, R), 0.0, 1.0), specular.w);
}
