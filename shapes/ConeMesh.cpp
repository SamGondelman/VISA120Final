#include "ConeMesh.h"

#include "gl/datatype/VAO.h"

/*
   a = atan(1/2) = about 26.6 degrees

   |\
   |a\ sqrt(5)/2
 1 |  \
   |   \
   +----+
    0.5
*/

#define SIN_ANGLE 0.447213595499958f
#define COS_ANGLE 0.894427190999916f
namespace {

    struct ConeMeshCallbackPoint {
        void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
            float sAngle = s * (2.0f * M_PI);
            float sSin = sinf(sAngle);
            float sCos = cosf(sAngle);
            float radius = 0.5f * t;
            normal = glm::vec3(COS_ANGLE * sSin, SIN_ANGLE, COS_ANGLE * sCos);
            position = glm::vec3(radius * sSin, 0.5f - t, radius * sCos);
            texture = glm::vec2(s-0.25, 1-t);
        }
    };

    ConeMeshCallbackPoint bodyGenFunc;

    struct ConeMeshCallbackTip {
        ConeMeshCallbackTip(int slices)
            : m_slices(slices) { }

        int m_slices;

        void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
            float sAngle = s * (2.0f * M_PI);
            float sSin = sinf(sAngle);
            float sCos = cosf(sAngle);
            float radius = 0.5f * t;
            position = glm::vec3(radius * sSin, 0.5f - t, radius * sCos);
            texture = glm::vec2(s-0.25, 1-t);
            if (t == 0.0f) {
                sAngle -= M_PI / m_slices;
                sSin = sinf(sAngle);
                sCos = cosf(sAngle);
            }
            normal = glm::vec3(COS_ANGLE * sSin, SIN_ANGLE, COS_ANGLE * sCos);
        }
    };

    struct ConeMeshCallbackBottom {
        void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
            float sAngle = s * (-2.0f * M_PI);
            float radius = t * 0.5f;
            position = glm::vec3(radius * sinf(sAngle), -0.5f, radius * cosf(sAngle));
            normal = glm::vec3(0, -1, 0);
            texture = glm::vec2(position.x + 0.5,position.z + 0.5);
        }
    };

    ConeMeshCallbackBottom bottomGenFunc;

    unsigned int sliceClamp(int slices) { return static_cast<unsigned int>(std::max(slices, 3)); }
    unsigned int stackClamp(int stacks) { return static_cast<unsigned int>(std::max(stacks, 1)); }
}

ConeMesh::ConeMesh(int stacks, int slices, float repeatU, float repeatV) :
    m_body(bodyGenFunc, sliceClamp(slices), stackClamp(stacks), 0, 1, sliceClamp(slices), stackClamp(stacks)),
    m_tip(ConeMeshCallbackTip(slices), sliceClamp(slices), stackClamp(stacks), 0, 0, sliceClamp(slices), 1, false),
    m_bottomCap(bottomGenFunc, sliceClamp(slices), stackClamp(stacks), 0, 0, sliceClamp(slices), stackClamp(stacks))
{
    m_repeatU = repeatU;
    m_repeatV = repeatV;
}

ConeMesh::~ConeMesh() {}

void ConeMesh::draw(int num) {
    m_body.draw(num);
    m_tip.draw(num);
    m_bottomCap.draw(num);
}
