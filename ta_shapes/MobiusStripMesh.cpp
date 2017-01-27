#include "MobiusStripMesh.h"

#include "gl/datatype/VAO.h"

namespace
{

struct MobiusSurfaceGenerator {
    void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
        s = s * 2.0f - 1.0f;
        t *= 4.0f * M_PI;

        // temporary variables to not repeat computations
        float temp1 = 0.5f * cosf(0.5f * t);
        float temp2 = 0.5f * s * -sinf(0.5f * t) * 0.5f;
        float temp3 = 0.5f + 0.25f * s * cosf(0.5f * t);

        glm::vec3 position_ds = glm::vec3 (
            temp1 * cosf(t),
            temp1 * sinf(t),
            0.5f * sinf(0.5f * t));
        glm::vec3 position_dt = glm::vec3 (
            temp2 * cosf(t) - temp3 * sinf(t),
            temp2 * sinf(t) + temp3 * cosf(t),
            0.5f * s * cosf(0.5f * t) * 0.5f);

        position = glm::vec3 (
            temp3 * cosf(t),
            temp3 * sinf(t),
            0.25f * s * sinf(0.5f * t));
        normal = glm::normalize(glm::cross(position_dt, position_ds));
        texture = glm::vec2(s,t);
    }
} genFunc;


unsigned int sliceClamp(int slice) { return static_cast<unsigned int>(std::max(slice, 3)); }
unsigned int stackClamp(int stack) { return static_cast<unsigned int>(std::max(stack, 1)); }

}

MobiusStripMesh::MobiusStripMesh(int stacks, int slices) :
    ParameterizedMesh(genFunc, stackClamp(stacks), 2 * sliceClamp(slices), 0, 0, stackClamp(stacks), 2 * sliceClamp(slices))
{
//    if (stacks < 1) stacks = 1;
//    if (slices < 3) slices = 3;

//    slices *= 2;
//    generateParameterizedSurface(mobiusStripMeshCallback, stacks, slices, 0, 0, stacks, slices);
}

MobiusStripMesh::~MobiusStripMesh() {}
