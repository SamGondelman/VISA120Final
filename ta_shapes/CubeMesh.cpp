#include "CubeMesh.h"

#include "glm/gtx/component_wise.hpp"

#include "gl/datatype/VAO.h"

struct cubeMeshCallbackXneg {
    void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
        position = glm::vec3(-0.5f, s - 0.5f, t - 0.5f);
        normal = glm::vec3(-1, 0, 0);
        texture = glm::vec2(t,s);
    }
} leftGenFunc;

struct cubeMeshCallbackXpos {
    void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
        position = glm::vec3(0.5f, 0.5f - s, t - 0.5f);
        normal = glm::vec3(+1, 0, 0);
        texture = glm::vec2(1.0f-t,1.0f-s);
    }
} rightGenFunc;

struct cubeMeshCallbackYneg {
    void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
        position = glm::vec3(0.5f - s, -0.5f, t - 0.5f);
        normal = glm::vec3(0, -1, 0);
        texture = glm::vec2(1.0f-s,t);
    }
} bottomGenFunc;

struct cubeMeshCallbackYpos {
    void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
        position = glm::vec3(s - 0.5f, 0.5f, t - 0.5f);
        normal = glm::vec3(0, +1, 0);
        texture = glm::vec2(s,1.0f-t);
    }
} topGenFunc;

struct cubeMeshCallbackZneg {
    void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
        position = glm::vec3(s - 0.5f, t - 0.5f, -0.5f);
        normal = glm::vec3(0, 0, -1);
        texture = glm::vec2(1.0f-s,t);
    }
} backGenFunc;

struct cubeMeshCallbackZpos {
    void operator()(float s, float t, glm::vec3 &position, glm::vec3 &normal, glm::vec2 &texture) {
        position = glm::vec3(0.5f - s, t - 0.5f, 0.5f);
        normal = glm::vec3(0, 0, +1);
        texture = glm::vec2(1.0f-s,t);
    }
} frontGenFunc;

namespace {
    unsigned int sideClamp(int side) { return static_cast<unsigned int>(std::max(side, 1)); }
}

CubeMesh::CubeMesh(int squaresPerEdge, float repeatU, float repeatV) :
    m_top(topGenFunc, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge), 0, 0, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge)),
    m_bottom(bottomGenFunc, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge), 0, 0, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge)),
    m_left(leftGenFunc, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge), 0, 0, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge)),
    m_right(rightGenFunc, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge), 0, 0, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge)),
    m_front(frontGenFunc, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge), 0, 0, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge)),
    m_back(backGenFunc, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge), 0, 0, sideClamp(squaresPerEdge), sideClamp(squaresPerEdge))
{
    m_repeatU = repeatU;
    m_repeatV = repeatV;
}

void CubeMesh::draw(int num) {
    m_top.draw(num);
    m_bottom.draw(num);
    m_left.draw(num);
    m_right.draw(num);
    m_front.draw(num);
    m_back.draw(num);
}

CubeMesh::~CubeMesh()
{
}
