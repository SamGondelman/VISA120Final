#version 400 core

out vec2 texCoord;

void main() {
    const float depth = 0.0;
    const vec4 UNIT_QUAD[4] = vec4[4](
        vec4(-1.0, 1.0, depth, 1.0),
        vec4(-1.0, -1.0, depth, 1.0),
        vec4(1.0, 1.0, depth, 1.0),
        vec4(1.0, -1.0, depth, 1.0)
    );
    vec4 pos = UNIT_QUAD[gl_VertexID];

    texCoord = (pos.xy + 1) * 0.5;
    gl_Position = pos;
}
