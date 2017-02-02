#include "SphereMesh.h"

#include <utility>

#include "glm/gtx/norm.hpp"

#include "gl/datatype/VAO.h"

struct SphereSurfaceGenerator {
    void operator()(float u, float v, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texCoord) {
        u = u * M_PI - M_PI/2;
        v *= 2*M_PI;
        normal = glm::vec3(cosf(u) * cosf(v), -sinf(u), cosf(u) * sinf(v));
        position = normal * 0.5f;
        texCoord = glm::vec2(1-v/M_PI/2,1-u/M_PI+0.5);
    }
};

namespace {

SphereSurfaceGenerator genFunc;

unsigned int sliceClamp(int slices) { return static_cast<unsigned int>(std::max(slices, 2)); }
unsigned int stackClamp(int slices) { return static_cast<unsigned int>(std::max(slices, 3)); }

}

SphereMesh::SphereMesh(int slices, int stacks, float repeatU, float repeatV) :
    ParameterizedMesh(genFunc, sliceClamp(slices), stackClamp(stacks), 0, 0, sliceClamp(slices), stackClamp(stacks))
{
    m_repeatU = repeatU;
    m_repeatV = repeatV;
}

SphereMesh::~SphereMesh() {}
