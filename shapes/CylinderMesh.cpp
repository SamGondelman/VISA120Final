#include "CylinderMesh.h"

#include "gl/datatype/VAO.h"

namespace {

struct CylinderMeshCallbackTop {
    void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
        float sAngle = s * (2.0f * M_PI);
        float radius = t * 0.5f;
        position = glm::vec3(radius * sinf(sAngle), 0.5f, radius * cosf(sAngle));
        normal = glm::vec3(0, +1, 0);
        texture = glm::vec2(sAngle,t);
        texture = glm::vec2(position.x + 0.5,0.5 - position.z);
    }
} topGenFunc;

struct CylinderMeshCallbackBarrel {
    void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
        float sAngle = s * (2.0f * M_PI);
        normal = glm::vec3(sinf(sAngle), 0.0f, cosf(sAngle));
        position = glm::vec3(0.5f * normal.x, 0.5f - t, 0.5f * normal.z);
        texture = glm::vec2(s - .25,1-t);
    }
} barrelGenFunc;

struct CylinderMeshCallbackBottom {
    void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
        float sAngle = s * (-2.0f * M_PI);
        float radius = t * 0.5f;
        position = glm::vec3(radius * sinf(sAngle), -0.5f, radius * cosf(sAngle));
        normal = glm::vec3(0, -1, 0);
        texture = glm::vec2(position.x + 0.5,position.z + 0.5);
    }
} bottomGenFunc;


unsigned int sliceClamp(int slices) { return static_cast<unsigned int>(std::max(slices, 3)); }
unsigned int stackClamp(int slices) { return static_cast<unsigned int>(std::max(slices, 1)); }

}

CylinderMesh::CylinderMesh(int stacks, int slices, float repeatU, float repeatV) :
    m_topCap(topGenFunc, sliceClamp(slices), stackClamp(stacks), 0, 0, sliceClamp(slices), stackClamp(stacks)),
    m_barrel(barrelGenFunc, sliceClamp(slices), stackClamp(stacks), 0, 0, sliceClamp(slices), stackClamp(stacks)),
    m_bottomCap(bottomGenFunc, sliceClamp(slices), stackClamp(stacks), 0, 0, sliceClamp(slices), stackClamp(stacks))
{
    m_repeatU = repeatU;
    m_repeatV = repeatV;
}

CylinderMesh::~CylinderMesh()
{
}

void CylinderMesh::draw(int num) {
    m_topCap.draw(num);
    m_barrel.draw(num);
    m_bottomCap.draw(num);
}
